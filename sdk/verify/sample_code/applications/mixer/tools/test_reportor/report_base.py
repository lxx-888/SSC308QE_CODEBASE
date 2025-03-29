#!/usr/bin/env python3
# coding=utf-8

import os
import json
import config as cfg


class ReportBase:
    CASE_RESULT_PASS=0
    CASE_RESULT_FAIL=1
    CASE_RESULT_NO_TEST=2
    CASE_RESULT_NO_NEED=3


    def __init__(self, dir_path: str):
        self.dir_path = dir_path


    def _before_run(self):
        pass


    def _add_menu(self, menu_path: str):
        print('add menu -> {}'.format(menu_path))


    def _add_case(self, menu_path: str, file_name: str, case_name: str, result_dict: dict):
        print("add case -> {}".format(case_name))


    def _after_run(self):
        pass


    def run(self):
        self._before_run()
        for root, _, files in os.walk(self.dir_path):
            root = os.path.normpath(root)
            root = root.replace('\\', '/')
            self._add_menu(root)
            for file in files:
                if not file.endswith('.json'):
                    continue
                file_name = os.path.splitext(file)[0]
                file_path = os.path.join(root, file)
                with open(file_path) as json_file:
                    data = json.load(json_file)
                    try:
                        case_name   = data[cfg.KEY_NAME_CASE_NAME]
                        result      = int(data[cfg.KEY_NAME_CASE_RESULT], 16)
                        stage       = int(data[cfg.KEY_NAME_CASE_STAGE], 16)
                        stage_range = int(data[cfg.KEY_NAME_CASE_STAGE_RANGE], 16)
                        result_dict = {}
                        for key, _ in cfg.STAGE_DEFINE.items():
                            if key & stage_range:
                                if key & stage:
                                    if key & result:
                                        result_dict[key] = self.CASE_RESULT_PASS
                                    else:
                                        result_dict[key] = self.CASE_RESULT_FAIL
                                else:
                                    result_dict[key] = self.CASE_RESULT_NO_TEST
                            else:
                                result_dict[key] = self.CASE_RESULT_NO_NEED
                        self._add_case(root, file_name, case_name, result_dict)
                    except KeyError:
                        print('json file {} format error'.format(file_path))
        self._after_run()


if __name__ == '__main__':
    report = ReportBase('./json_out/')
    report.run()
    pass
