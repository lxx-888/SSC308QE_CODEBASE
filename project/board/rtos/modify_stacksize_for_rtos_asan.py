import re
import sys

size_table=[
    "CONFIG_STACK_SIZE_BOOT",
    "CONFIG_STACK_SIZE_ABT",
    "CONFIG_STACK_SIZE_UND",
    "CONFIG_TASK_CUST_STACK_SIZE"
]

def update_stack_size(file_path, should_update):
    if should_update:
        try:
            #### enlarge stack size
            with open(file_path, 'r') as file:
                content = file.read()

            for string in size_table:
                print(string)
                # regular match STACK_SIZE partterns.
                stack_size_pattern = re.compile(r'{}=(0x[0-9A-Fa-f]+)'.format(string))
                match = stack_size_pattern.search(content)
                if match:
                    # get current value
                    original_value = int(match.group(1), 16)
                    # x2
                    new_value = original_value * 2
                    # hex format
                    new_value_hex = format(new_value, 'X')
                    # replace
                    content = stack_size_pattern.sub('{}=0x{}'.format(string, new_value_hex), content)
                # write back
                with open(file_path, 'w') as file:
                    file.write(content)
            ##### enable CONFIG_RTOS_MCU_ASAN_SUPPORT
            with open(file_path, 'r') as file:
                lines = file.readlines()  # read all line

            # regular match line 'CONFIG_RTOS_MCU_ASAN_SUPPORT'
            mcu_support_pattern = re.compile(r'.*CONFIG_RTOS_MCU_ASAN_SUPPORT.*')

            # check if there's 'CONFIG_RTOS_MCU_ASAN_SUPPORT' in file
            if any(mcu_support_pattern.search(line) for line in lines):
                # regular replace
                new_lines = [mcu_support_pattern.sub('CONFIG_RTOS_MCU_ASAN_SUPPORT=y', line) for line in lines]
            else:
                # if can't find, add 'MCU_SUPPORT=y' to end of file
                new_lines = lines + ['\nCONFIG_RTOS_MCU_ASAN_SUPPORT=y\n']
            # write back changed content.
            with open(file_path, 'w') as file:
                file.writelines(new_lines)

        except FileNotFoundError:
            print("Error: The file '{}' does not exist.".format(file_path))
        except IOError as e:
            print("An I/O error occurred: {}".format(e.strerror))
        except Exception as e:
            print("An error occurred: {}".format(e))

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python script.py <file_path> <true|false>")
    else:
        file_path = sys.argv[1]
        should_update = sys.argv[2].lower() == 'true'
        update_stack_size(file_path, should_update)