
import sys
import os
from pathlib import Path

def main(resource_files):
    rc_file_path = Path("resources.rc")
    header_file_path = Path("resource.h")

    # 确定新资源的起始ID
    start_id = 101
    if header_file_path.exists():
        with open(header_file_path, "r") as header_file:
            for line in header_file:
                if "#define" in line:
                    parts = line.split()
                    if len(parts) >= 3 and parts[1].startswith("IDR_"):
                        current_id = int(parts[2])
                        start_id = max(start_id, current_id + 1)

    # 打开资源文件和头文件，准备追加内容
    with open(rc_file_path, "a") as rc_file, open(header_file_path, "a") as header_file:
        for resource_file in resource_files:
            filename = Path(resource_file).stem.upper()
            resource_id = f"IDR_{filename}"
            resource_path = Path(resource_file).resolve()

            # 写入资源文件
            rc_entry = f'{resource_id} RCDATA "{resource_path}"\n'
            rc_file.write(rc_entry)

            # 写入头文件
            header_entry = f"#define {resource_id} {start_id}\n"
            header_file.write(header_entry)

            start_id += 1

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python add_resource.py <file1> [<file2> ...]")
    else:
        main(sys.argv[1:])
