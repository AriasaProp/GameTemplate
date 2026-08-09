// #define NOBDEBUG "-ggdb", 
#define NOBDEF extern
#define NOB_NO_ECHO
#include "nob_extra.c"
#include "help.h"
#include "config.h"

#define NOB_IMPLEMENTATION
#include "nob.h"
#define GREEN "\033[32m"
#define RED "\033[31m"
#define RESET "\033[0m"
#define  BIN_DIR           "bin"
#define  OBJ_DIR   BIN_DIR"/obj"
#define TOOL_DIR  BIN_DIR"/tool"
#define TEST_DIR  BIN_DIR"/test"

enum : int {
  ActionFlags_DebugRun,
  ActionFlags_ForceBuild,
};

typedef struct {
  const char *src;
  File_Paths deps;
} File_Src;

#define COMMON_SRC "main/common.c"
#define STB_SRCS \
  COMMON_SRC, \
	"ext/stb/image_read.c", \
	"ext/stb/image_write.c", \
	"ext/stb/truetype.c", \
	"ext/stb/rectpack.c", \
	"ext/stb/local.c"

typedef struct {
  const char *name;
  const char **srcs;
} File_Exe;

static const File_Exe Test_Execs[] = {
  {
    .name = "hello",
    .srcs = CLIT(const char *[]) {
		  "tests/hello.c",
      NULL
    }
  },
};
static const File_Exe Tool_Execs[] = {
  {
    .name = "hello",
    .srcs = CLIT(const char *[]) {
		  "tools/hello.c",
      NULL
    }
  },
};
#undef COMMON_SRC
#undef STB_SRCS

static Cmd cmd;
static Procs procs;
static int actionFlags;

typedef struct {
  const char **items;
  size_t count, capacity;
} Task;

// group function
static void info_ask    (void);
static bool clean_group (Task*);
static void status_group(Task*);
static bool test_group  (Task*);
static bool tool_group  (Task*);
// root function
static bool exec_run(const char *);
static int  obj_compile(const char*, const char**);
static bool exec_compile(const char*, const char**,const char**);
static bool walk_dir_cleanup(Walk_Entry);

int main(int argc, char **argv) {
  GO_REBUILD_URSELF_PLUS(argc, argv, "run/nob.h", "run/nob_extra.c", "run/config.h", "run/help.h");
  shift(argv, argc);
  Task task = {0};
  for (;argc;shift(argv, argc)) {
    if ((*argv)[0] == '-') {
      if ((*argv)[1] == '-') {
        const char *ar = (*argv) + 2;
        // double dash flags
        if (!strcmp("debug", ar)) {
          actionFlags |= ActionFlags_DebugRun;
        } else if (!strcmp("build", ar)) {
          actionFlags |= ActionFlags_ForceBuild;
        }
      } else {
        // dash flags
        for (const char *ar = *argv; *(++ar); ) {
          switch (*ar) {
            case 'd': actionFlags |= ActionFlags_DebugRun; break;
            case 'b': actionFlags |= ActionFlags_ForceBuild; break;
            default: break;
          }
        }
      }
    } else {
      da_append(&task, *argv);
    }
  }
  int ret = EXIT_SUCCESS;
#define CASE_ACT(A,B) if (!strcmp(da_first(&task), A) || !strcmp(da_first(&task), B))
  if (!task.count) {
    nob_log(NOB_ERROR, "at least give a command.\n");
    printf (help_msg);
    ret = EXIT_FAILURE;
  } else {
    CASE_ACT("h","help") {
      printf (help_msg);
    } else CASE_ACT("c","clean") {
      da_remove_first_item(&task);
      if (!clean_group(&task)) ret = EXIT_FAILURE;
    } else CASE_ACT("s","status") {
      da_remove_first_item(&task);
      status_group(&task);
    } else CASE_ACT("i","info") {
      da_remove_first_item(&task);
      // info print out
      info_ask();
    } else if (!strcmp(da_first(&task), "test")) {
      da_remove_first_item(&task);
      if (!test_group(&task)) ret = EXIT_FAILURE;
    } else if (!strcmp(da_first(&task), "tool")) {
      da_remove_first_item(&task);
      if (!tool_group(&task)) ret = EXIT_FAILURE;
    } else {
      nob_log(NOB_ERROR, "%s option doesn't exists.\n", da_first(&task));
      printf (help_msg);
      ret = EXIT_FAILURE;
    }
  }
#undef CASE_ACT
  da_free(task);
  da_free(procs);
  da_free(cmd);
  return ret;
}

static void info_ask(void) {
  // Informasi OS
#if defined(_WIN64)
  nob_log(NOB_INFO, "OS: Windows 64-bit");
#elif defined(_WIN32)
  nob_log(NOB_INFO, "OS: Windows 32-bit");
#elif defined(__linux__)
  nob_log(NOB_INFO, "OS: Linux");
#elif defined(__APPLE__) && defined(__MACH__)
  nob_log(NOB_INFO, "OS: macOS");
#elif defined(__unix__)
  nob_log(NOB_INFO, "OS: Unix");
#else
  nob_log(NOB_INFO, "OS: Unknown");
#endif
  // Informasi compiler
#if defined(__clang__)
  nob_log(NOB_INFO, "Compiler: Clang %d.%d.%d", __clang_major__, __clang_minor__, __clang_patchlevel__);
#elif defined(__GNUC__) || defined(__GNUG__)
  nob_log(NOB_INFO, "Compiler: GCC %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#elif defined(_MSC_VER)
  nob_log(NOB_INFO, "Compiler: MSVC %d", _MSC_VER);
#else
  nob_log(NOB_INFO, "Compiler: Unknown");
#endif

  // Informasi arsitektur
  nob_log(NOB_INFO, "Architecture: "
#if defined(__x86_64__) || defined(_M_X64)
  	"x86_64 (64-bit)"
#elif defined(__i386) || defined(_M_IX86)
  	"x86 (32-bit)"
#elif defined(__aarch64__)
  	"ARM64 (64-bit)"
#elif defined(__arm__) || defined(_M_ARM)
  	"ARM (32-bit)"
#else
		"Unknown"
#endif
	);
  // Informasi byte order
#ifdef BYTE_ORDER
  nob_log(NOB_INFO, "Byte order: "
#  if BYTE_ORDER == LITTLE_ENDIAN
		"Little"
#  else
		"Big"
#  endif // BYTE_ORDER
		" Endian (BYTE_ORDER)");
#elif defined(__BYTE_ORDER__)
  nob_log(NOB_INFO, "Byte order: "
#  if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  	"Little"
#  else
  	"Big"
#  endif // __BYTE_ORDER__
		" Endian (__BYTE_ORDER__)");
#else
  {
    unsigned int x = 1;
    char *c = (char*)&x;
  	nob_log(NOB_INFO, "Byte order: %s Endian (Manual)", *c == 1 ? "Little" : "Big");
  }
#endif
}
static bool clean_group(Task *task) {
  bool result = true;
  int group = 0;
  while (task->count) {
    if (!strcmp(da_first(task), "test"))
      group |= 1;
    else if (!strcmp(da_first(task), "tool"))
      group |= 2;
    da_remove_first_item(task);
  }
  if (!group) group = 3;
  if (group & 1) {
  	if (!file_exists(TEST_DIR))
  	  nob_log(NOB_INFO, "Binary test file wasn't exists.");
  	else if (walk_dir(TEST_DIR, walk_dir_cleanup, .post_order = true))
  	  nob_log(NOB_INFO, "Cleanup binary test walk is suceed.");
  	else {
  	  nob_log(NOB_ERROR, "Cleanup binary test walk is error.");
      result = false;
    }
  }
  if (result && (group & 2)) {
  	if (!file_exists(TOOL_DIR))
  	  nob_log(NOB_INFO, "Binary tool file wasn't exists.");
  	else if (walk_dir(TOOL_DIR, walk_dir_cleanup, .post_order = true))
  	  nob_log(NOB_INFO, "Cleanup binary tool walk is suceed.");
    else {
  	  nob_log(NOB_ERROR, "Cleanup binary tool walk is error.");
      result = false;
    }
  }
  return result;
}
static void status_group(Task *task) {
  size_t i, j;
  int group = 0;
  bool exists;
  while (task->count) {
    if (!strcmp(da_first(task), "test"))
      group |= 1;
    else if (!strcmp(da_first(task), "tool"))
      group |= 2;
    da_remove_first_item(task);
  }
  if (!group) group = 3;
  if (group & 1) {
    for (i = 0; i < ARRAY_LEN(Test_Execs); ++i) {
      j = temp_save();
      exists = file_exists(temp_sprintf(TEST_DIR"/%s", Test_Execs[i].name));
      temp_rewind(j);
      nob_log(NOB_INFO, "Test %s exec is %s" RESET ".", Test_Execs[i].name, (exists ? GREEN "passed" RESET : RED "not pass" RESET));
    }
  }
  if (group & 2) {
  	for (i = 0; i < ARRAY_LEN(Tool_Execs); ++i) {
      j = temp_save();
      exists = file_exists(temp_sprintf(TOOL_DIR"/%s", Tool_Execs[i].name));
      temp_rewind(j);
      nob_log(NOB_INFO, "Tool %s exec is %s.", Tool_Execs[i].name, (exists ? GREEN "passed" RESET : RED "not pass" RESET));
    }
  }
}
static bool test_group(Task *task) {
  size_t i, j;
  bool result = true;
  const char *compile_flags[] = {
#ifdef _MSC_VER
    "/Od", "/Zi", "/I.\tests", "/I.\ext", "/I.\tools",
#else                   
    "-O0", "-ggdb", "-I./tests", "-I./ext", "-I./tools",
#endif
		NULL
  };
  if (!task->count) {
    nob_log(NOB_INFO, "Compile & running All Tests");
    for (i = 0; result && (i < ARRAY_LEN(Test_Execs)); ++i) {
      nob_log(NOB_INFO, "Compile & running %s", Test_Execs[i].name);
      j = temp_save();
      const char *out = temp_sprintf(TEST_DIR"/%s", Test_Execs[i].name);
      result &= exec_compile(out, Test_Execs[i].srcs, compile_flags) && exec_run(out);
      temp_rewind(j);
    }
  } else {
    for (i = 0; i < ARRAY_LEN(Test_Execs); ++i)
      if (!strcmp(da_first(task), Test_Execs[i].name))
        break;
    if (i < ARRAY_LEN(Test_Execs)) {
      nob_log(NOB_INFO, "Compile & running %s", Test_Execs[i].name);
      j = temp_save();
      const char *out = temp_sprintf(TEST_DIR"/%s", Test_Execs[i].name);
      result = exec_compile(out, Test_Execs[i].srcs, compile_flags) && exec_run(out);
      temp_rewind(j);
    } else {
      nob_log(NOB_ERROR, "Unknown test of %s", da_first(task));
      result = false;
    }
  }
  return result;
}
static bool tool_group(Task *task) {
  size_t i, j;
  bool result = true;
  const char *compile_flags[] = {
#ifdef _MSC_VER
    "/O3", "/I.\ext", "/I.\tool",
#else                   
    "-O3", "-I./ext", "-I./tool",
#endif
		NULL
  };
  if (!task->count) {
    nob_log(NOB_INFO, "Compile All Tools");
    for (i = 0; result && (i < ARRAY_LEN(Tool_Execs)); ++i) {
      nob_log(NOB_INFO, "Compile %s", Tool_Execs[i].name);
      j = temp_save();
      const char *out = temp_sprintf(TOOL_DIR"/%s", Tool_Execs[i].name);
      result &= exec_compile(out, Tool_Execs[i].srcs, compile_flags);
      temp_rewind(j);
    }
  } else {
    for (i = 0; i < ARRAY_LEN(Tool_Execs); ++i)
      if (!strcmp(da_first(task), Tool_Execs[i].name))
        break;
    if (i < ARRAY_LEN(Tool_Execs)) {
      nob_log(NOB_INFO, "Compile %s", Tool_Execs[i].name);
      j = temp_save();
      const char *out = temp_sprintf(TOOL_DIR"/%s", Tool_Execs[i].name);
      result = exec_compile(out, Tool_Execs[i].srcs, compile_flags);
      temp_rewind(j);
    } else {
      nob_log(NOB_ERROR, "Unknown tool of %s", da_first(task));
      result = false;
    }
  }
  return result;
}
static bool exec_run(const char *exec) {
  if (actionFlags & ActionFlags_DebugRun) {
#ifdef __linux__
    cmd_append(&cmd, "gdb");
#else                   
    nob_log(NOB_INFO, "how to debug on other platform? just run without debug");
#endif
  }
  cmd_append(&cmd, exec);
  if (!cmd_run(&cmd)) {
    delete_file(exec);
    return false;
  }
#ifdef __linux__
  if (actionFlags & ActionFlags_DebugRun)
    delete_file(exec);
#endif
  return true;
}
static bool exec_compile(const char *out, const char **srcs, const char **flags) {
  size_t point = temp_save(), i, j;
  // exec need rebuild ?
  {
    bool rebuild = !file_exists(out);
    int build;
    for(i = 0; srcs[i]; ++i) {
      build = obj_compile(srcs[i], flags);
      if (build < 0) {
        nob_log(NOB_ERROR, "make objs %s for executable %s is fail", srcs[i], out);
        if (!procs_flush(&procs))
          nob_log(NOB_ERROR, "fail procs compile %s", out);
        return false;
      }
      rebuild = rebuild || !!build;
    }
    if (!rebuild) return true;
  }
  if (!mkdir_rec(temp_dir_name(out)))
    return false;
  // Wait on all the async processes to finish and reset procs dynamic array to 0
  if (!procs_flush(&procs)) {
    nob_log(NOB_ERROR, "fail procs compile %s", out);
    return false;
  }
  nob_cc(&cmd);
  // append objs file
  for(i = 0; srcs[i]; ++i)
    da_append(&cmd, temp_sprintf(OBJ_DIR"/%s.o", srcs[i]));
  nob_cc_output(&cmd, out);
#ifndef _MSC_VER
  cmd_append(&cmd, 
   "-lc",
# ifndef NO_STDMATH
   "-lm",
# endif
  );
#endif
	temp_rewind(point);
  return cmd_run(&cmd);
}

static int obj_compile(const char *in, const char **flags) {
  size_t point = temp_save(), j, k;
  const char *out = temp_sprintf(OBJ_DIR"/%s.o", in);
  int res = (actionFlags & ActionFlags_ForceBuild);
  if (res < 1)
  	res = !file_exists(out);
  if (res < 1) {
    // load dependencies
    const char *depen_file = temp_sprintf(OBJ_DIR"/%s.d", in);
		String_Builder sb = {0};
    if (file_exists(depen_file) && read_entire_file(depen_file, &sb)) {
      File_Paths fp = {0};
      String_View sv = sb_to_sv(sb);
      String_View src = sv_chop_by_delim(&sv, ':');
      for (j = 0; (j < sv.count); ++j) {
        if (!sv.data[j] || isspace(sv.data[j]) || (sv.data[j] == '\\'))
          continue;
        for (k = j + 1; (k < sv.count) && (!isspace(sv.data[k]) &&
            (sv.data[k] != '\\') && sv.data[k]
          ); ++k) ;
        da_append(&fp, nob_temp_strndup(sv.data + j, k - j));
        j = k;
      }
      res = needs_rebuild(out, fp.items, fp.count);
      da_free(fp);
    }
    // compile it anyway
  }
  if (res > 0)
    res = 1 - 2 * !mkdir_rec(temp_dir_name(out));
  if (res > 0) {
    // res == 1, let's build
    // create obj file
    char *ext = temp_file_ext(in);
    if (!strcmp(ext, ".c")) {
      nob_cc(&cmd);
  #ifdef _MSC_VER
  # error("object input cl.exe")
  #else
      cmd_append(&cmd, "-c", in);
  #endif
      nob_cc_output(&cmd, out);
      cmd_append(&cmd,
  #ifdef _MSC_VER
        "/MMD", "/std:c11", "/TP", "/WX", "/W4", "/nologo", "/D_CRT_SECURE_NO_WARNINGS", "/I.\main",
  #  ifdef NO_STDMATH
        "/DNO_STDMATH",
  #  endif // NO_STDMATH
  #  ifdef FAST_MATH
        "/fp:fast", "/DFASTER_MATH",
  #  endif // FAST_MATH
  #else
        "-MMD", "-std=c11", "-Werror", "-Wall", "-I./main",
  #  ifdef NO_STDMATH
        "-DNO_STDMATH",
  #  endif // NO_STDMATH
  #  ifdef FAST_MATH
        "-ffast-math", "-DFASTER_MATH",
  #  endif // FAST_MATH
  #endif
      );
      for (j = 0; flags[j]; ++j) da_append(&cmd, flags[j]);
    } else {
      nob_log(NOB_ERROR, "not ready to compile %s file", ext);
      res = -1;
    }
    if (res > 0) res = cmd_run(&cmd, .async = &procs) ? 1 : -1;
  }
  temp_rewind(point);
  return res;
}
static bool walk_dir_cleanup(Walk_Entry entry) {
  return delete_file(entry.path);
}

