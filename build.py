import os
import shutil
import argparse
import subprocess


def subprocess_call(*args):
    assert len(args) > 0

    print(f"[[RUNNING]] : {' '.join(args)}")

    res = subprocess.call(args)

    if res != 0:
        print(f"[ERR] {str(args[0])} execution failed : {res}")
        exit(res)


if __name__ == "__main__":
    # CLI

    ap = argparse.ArgumentParser(description="[bretema] - source compile helper")
    ap.add_argument("-c", "--clean", default=False, action="store_true", help="perform a cleanup before anything")
    ap.add_argument("-b", "--build", default=False, action="store_true", help="enables build")
    ap.add_argument("-d", "--debug", default=False, action="store_true", help="compile as debug (default is release)")
    ap.add_argument("-t", "--tests", default=False, action="store_true", help="also compile tests")
    ap.add_argument("-v", default=False, action="store_true", help="enable cmake log-level : 'status'")
    args = ap.parse_args()

    # Parse arguments

    build_tests_str = "ON" if args.tests else "OFF"
    build_type_str = "Debug" if args.debug == 1 else "Release"

    log_level_str = "STATUS" if args.v else "WARNING"

    args_msg_tmpl = "[args] => clean:{}, build:{}, tests:{}, debug:{}, verbose:{}"
    args_msg = args_msg_tmpl.format(args.clean, args.build, args.tests, args.debug, log_level_str)
    print(args_msg)

    # Create build folder

    path = "./build/"

    if not os.path.exists(path):
        print("\n[mkdir] => {}".format(path))
        os.mkdir(path)

    # Clean (remove files from ./build/)

    if args.clean:
        print("\n[cleaning] => {}".format(path))

        for root, dirs, files in os.walk(path):
            print("  [cleaning] => FILES")
            for f in files:
                file = os.path.join(root, f)
                print("    -> {}".format(file))
                os.unlink(file)

            print("  [cleaning] => DIRS")
            for d in dirs:
                dir = os.path.join(root, d)
                print("    -> {}".format(dir))
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
            "Ninja",
            "..",
        )

        subprocess_call("ninja")

        print(f"\n{'*'*60}\n")
