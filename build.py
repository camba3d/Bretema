import os
import shutil
import argparse


# TODO: UPDATE THIS TO USE 'subprocess' NOT 'os.system'... *facepalm*

if __name__ == "__main__":

    # CLI

    ap = argparse.ArgumentParser(description="Project Builder...")
    ap.add_argument("--build", default=False,
                    action=argparse.BooleanOptionalAction)
    ap.add_argument("--clean", default=False,
                    action=argparse.BooleanOptionalAction)
    ap.add_argument("--tests", default=False,
                    action=argparse.BooleanOptionalAction)
    ap.add_argument("--debug", default=False,
                    action=argparse.BooleanOptionalAction)
    ap.add_argument("--release", default=False,
                    action=argparse.BooleanOptionalAction)
    args = ap.parse_args()

    if args.release and args.debug:
        exit("[ERR] => Select only one '--debug' or '--release'")

    # Parse arguments

    build_tests_str = "ON" if args.tests else "OFF"
    build_type_str = "Release" if args.release == 1 else "Debug"

    args_msg_tmpl = "[args] => clean:{}, build:{}, tests:{}, type:{}"
    args_msg = args_msg_tmpl.format(
        args.clean, args.build, build_tests_str, build_type_str)
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

        cmake_cmd_tmpl = 'cmake --log-level=WARNING -DCMAKE_BUILD_TYPE={} .. -G "Ninja" -DOPT_TESTS={}'
        cmake_cmd = cmake_cmd_tmpl.format(build_type_str, build_tests_str)

        print("\n[cmake] => {}".format(cmake_cmd))
        os.system(cmake_cmd)

        print("\n[ninja] => {}".format(path))
        os.system("ninja")

        print("\n")
