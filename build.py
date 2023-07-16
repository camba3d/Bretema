import os
import shutil
import argparse
import subprocess


def add_to_path(path):
    """Adds the given path to the system path."""
    os.environ["PATH"] += ";" + path


def subprocess_call(*args):
    assert len(args) > 0

    print(f"[[RUNNING]] : {' '.join(args)}")

    res = subprocess.call(args)

    if res != 0:
        print(f"[ERR] {str(args[0])} execution failed : {res}")
        exit(res)


def main():
    # PRE

    add_to_path(os.path.join(os.environ["ProgramFiles(x86)"], "Microsoft Visual Studio", "2019", "Community", "MSBuild", "Current", "Bin"))

    # CLI

    ap = argparse.ArgumentParser(description="[bretema] - source compile helper")
    ap.add_argument("-c", "--clean", default=False, action="store_true", help="perform a cleanup before anything")
    ap.add_argument("-b", "--build", default=False, action="store_true", help="enables build")
    ap.add_argument("-d", "--debug", default=False, action="store_true", help="compile as debug (default is release)")
    ap.add_argument("-t", "--tests", default=False, action="store_true", help="also compile tests")

    # ap.add_argument("--ninja", default=False, action="store_true", help="use NINJA as CMake-Generator")
    ap.add_argument("--msvc19", default=False, action="store_true", help="use MSVC2019 as CMake-Generator")

    ap.add_argument("-v", default=False, action="store_true", help="enable cmake log-level : 'status'")
    args = ap.parse_args()

    gen = "Visual Studio 16 2019" if args.msvc19 else "Ninja"

    def verb_print(msg):
        if args.v:
            print(msg)

    # Parse arguments

    build_tests_str = "ON" if args.tests else "OFF"
    build_type_str = "Debug" if args.debug == 1 else "Release"
    build_cmd = f"msbuild ALL_BUILD.vcxproj /p:Configuration={build_type_str}" if args.msvc19 else "Ninja"

    log_level_str = "STATUS" if args.v else "WARNING"

    args_msg_tmpl = "[args] => clean:{}, build:{}, tests:{}, debug:{}, verbose:{}"
    args_msg = args_msg_tmpl.format(args.clean, args.build, args.tests, args.debug, log_level_str)
    print(args_msg)

    # Create build folder

    path = "./build/"

    if not os.path.exists(path):
        verb_print("\n[mkdir] => {}".format(path))
        os.mkdir(path)

    # Clean (remove files from ./build/)

    if args.clean:
        verb_print("\n[cleaning] => {}".format(path))

        for root, dirs, files in os.walk(path):
            verb_print("  [cleaning] => FILES")
            for f in files:
                file = os.path.join(root, f)
                verb_print("    -> {}".format(file))
                os.unlink(file)

            verb_print("  [cleaning] => DIRS")
            for d in dirs:
                dir = os.path.join(root, d)
                verb_print("    -> {}".format(dir))
                shutil.rmtree(dir)

    # Build (cmake)

    if args.build:
        os.chdir(path)

        subprocess_call(
            "cmake",
            f"--log-level={log_level_str}",
            f"-DCMAKE_BUILD_TYPE={build_type_str}",
            f"-DOPT_TESTS={build_tests_str}",
            "-G",
            gen,
            "..",
        )

        subprocess_call(*build_cmd.split(" "))


if __name__ == "__main__":
    try:
        main()
        print(f"\n{'*'*60}\n")
    except KeyboardInterrupt:
        print("\n[INFO] : User interruptus")
        exit(0)
    except Exception as e:
        print(f"\n[CRITICAL] : {e}")
        exit(1)
