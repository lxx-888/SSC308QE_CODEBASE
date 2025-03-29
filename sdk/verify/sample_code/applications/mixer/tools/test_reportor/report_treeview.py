#!/usr/bin/env python3
# coding=utf-8

import config as cfg
from report_base import ReportBase
from tkinter import ttk


class ReportTreeView(ReportBase):
    TREE_COLUMNS = ['pass', 'fail', 'no_test']
    TREE_COLUMNS_NAME = ['成功', '失败', '待测试']


    class Counter:
        def __init__(self) -> None:
            self.success = 0
            self.fail = 0
            self.no_test = 0


        def add(self, result: int):
            if result == ReportTreeView.CASE_RESULT_PASS:
                self.success += 1
            elif result == ReportTreeView.CASE_RESULT_FAIL:
                self.fail += 1
            elif result == ReportTreeView.CASE_RESULT_NO_TEST:
                self.no_test += 1


    def __init__(self, json_out_dir: str, tree: ttk.Treeview, stage: int):
        super().__init__(json_out_dir)
        self.tree = tree
        self.stage = stage
        self.tree_item_dict = {}
        self.tree_item_cnt = {}


    def _before_run(self):
        pass


    def _add_menu(self, menu_path: str):
        parent = ''
        idx = menu_path.rfind('/')
        if idx != -1:
            parent = menu_path[:idx]
        self.tree_item_cnt[menu_path] = ReportTreeView.Counter()
        if self.tree_item_dict.get(menu_path) is None:
            self.tree_item_dict[menu_path] = \
                self.tree.insert(parent, 'end', iid=menu_path, text=menu_path[idx+1:], values=[])
            return


    def _add_case(self, menu_path: str, file_name: str, case_name: str, result_dict: dict):
        values = ['√' if result_dict[self.stage] == self.CASE_RESULT_PASS else '',
                  '√' if result_dict[self.stage] == self.CASE_RESULT_FAIL else '',
                  '√' if result_dict[self.stage] == self.CASE_RESULT_NO_TEST else '']
        curr_path = menu_path
        while True:
            self.tree_item_cnt[curr_path].add(result_dict[self.stage])
            idx = curr_path.rfind('/')
            if idx == -1:
                break
            curr_path = curr_path[:idx]

        if self.tree_item_dict.get(menu_path + case_name) is None:
            self.tree_item_dict[menu_path + case_name] = \
                self.tree.insert(menu_path, 'end', iid=menu_path + case_name, text=case_name, values=values)
            return
        self.tree.item(menu_path + case_name, values=values)


    def _after_run(self):
        for tree_item, cnt in self.tree_item_cnt.items():
            self.tree.item(tree_item, values=[cnt.success, cnt.fail, cnt.no_test])
        pass


if __name__ == '__main__':
    import tkinter as tk
    window = tk.Tk()

    screen_width = window.winfo_screenwidth()
    screen_height = window.winfo_screenheight()

    window.title('mixer report')
    window.geometry('{:d}x{:d}+{:d}+{:d}'.format(
        int(screen_width / 2), int(screen_height / 1.5),
        int(screen_width / 4), int(screen_height / 6)))
    window.resizable(0, 0)
    tree = ttk.Treeview(window, columns=ReportTreeView.TREE_COLUMNS)

    for col, name in zip(ReportTreeView.TREE_COLUMNS, ReportTreeView.TREE_COLUMNS_NAME):
        tree.column(col, anchor='center')
        tree.heading(col, text=name)

    report = ReportTreeView('./pipeline_i6f/', tree, 0xff)
    report.run()

    #refresh_tree(data, tree)
    tree.pack(expand = True, fill = tk.BOTH)

    window.mainloop()
    pass
