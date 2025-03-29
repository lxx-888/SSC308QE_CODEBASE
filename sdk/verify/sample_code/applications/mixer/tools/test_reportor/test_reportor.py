import os
import tkinter as tk
from tkinter import ttk
from tkinter import filedialog
from tkinter import messagebox
from report_treeview import ReportTreeView
from report_excel import ReportExcel
import config as cfg

class Application(tk.Frame):
    def __init__(self, master=None):
        super().__init__(master)
        self.master = master
        self.master.title("Mixer测试报告")

        # 获取屏幕的宽度和高度
        screen_width = self.master.winfo_screenwidth()
        screen_height = self.master.winfo_screenheight()

        # 将窗口大小设置为屏幕大小的一半
        self.master.geometry(f"{screen_width // 2}x{screen_height // 2}")

        # 创建 TreeView
        self.tree = ttk.Treeview(self, columns=ReportTreeView.TREE_COLUMNS)
        for col, name in zip(ReportTreeView.TREE_COLUMNS, ReportTreeView.TREE_COLUMNS_NAME):
            self.tree.column(col, anchor='center')
            self.tree.heading(col, text=name)
        self.tree.pack(fill=tk.BOTH, expand=True)

        # 创建按钮框架
        button_frame = tk.Frame(self)
        button_frame.pack(side=tk.BOTTOM, fill=tk.X)

        # 创建加载按钮
        self.load_button = tk.Button(button_frame, text="加载", command=self.load)
        self.load_button.pack(side=tk.LEFT, padx=10, pady=10, fill=tk.X, expand=True)
        self.json_dir_path = ''

        # 创建刷新按钮
        self.refresh_button = tk.Button(button_frame, text="刷新", command=self.refresh)
        self.refresh_button.pack(side=tk.LEFT, padx=10, pady=10, fill=tk.X, expand=True)

        # 创建导出按钮
        self.export_button = tk.Button(button_frame, text="导出", command=self.export)
        self.export_button.pack(side=tk.LEFT, padx=10, pady=10, fill=tk.X, expand=True)

        # 创建测试阶段下拉框
        self.stage_options = { val: key for key, val in cfg.STAGE_DEFINE.items() }
        keys = list(self.stage_options.keys())
        self.stage_select = keys[0]
        self.stage_var = tk.StringVar(value=keys[0])
        self.stage_dropdown = ttk.Combobox(button_frame, textvariable=self.stage_var, values=keys)
        self.stage_dropdown.bind("<<ComboboxSelected>>", self.stage_selected)
        if len(keys) == 1:
            self.stage_dropdown.configure(state="disabled")
        self.stage_dropdown.pack(side=tk.LEFT, padx=10, pady=10, fill=tk.X, expand=True)

        self.pack(fill=tk.BOTH, expand=True)

        self.report_tree = None

    def refresh(self):
        if self.report_tree:
            self.report_tree.run()

    def export(self):
        if not self.report_tree:
            return

        file_path = filedialog.asksaveasfilename(defaultextension='.xlsx')
        if not file_path:
            return

        try:
            report_excel = ReportExcel(self.json_dir_path, file_path)
            report_excel.run()
        except:
            messagebox.showerror(title='导出失败', message='导出失败')
            return

        messagebox.showinfo(title='导出成功', message='导出成功')

    def load(self):
        self.json_dir_path = filedialog.askdirectory()
        os.chdir(os.path.dirname(self.json_dir_path))
        self.json_dir_path = os.path.relpath(self.json_dir_path)
        self.report_tree = ReportTreeView(self.json_dir_path, self.tree, self.stage_options[self.stage_select])
        self.tree.delete(*self.tree.get_children())
        self.refresh()

    def stage_selected(self, event):
        self.stage_select = self.stage_options[self.stage_var.get()]
        self.refresh()
        pass

root = tk.Tk()
app = Application(master=root)
app.mainloop()
