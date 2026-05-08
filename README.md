# VLSI-Partition

VLSI 课程大作业代码仓库 — **图划分算法实验（实验一）**。

对应课程网页[链接](https://customized-computing.github.io/VLSI-FPGA/)。

## 内容

本仓库为课程实验一（图划分算法实践）及其大作业进阶要求的代码实现。

- [实验一基础要求](https://customized-computing.github.io/VLSI-FPGA/#/lab1/lab1_problem)
- [实验一进阶要求（大作业）](https://customized-computing.github.io/VLSI-FPGA/#/lab1/lab1_problem_advanced)
- [大作业说明](https://customized-computing.github.io/VLSI-FPGA/#/labFinal)

## 🔨 开发人员

> 面向后面接手这个项目的助教，或其他参考本项目的课程代码设计实践。

## 文档 Web 搭建

采用 [docsify](https://docsify.js.org/) 部署文档静态网页。<br>
需要使用 NPM 进行安装，[NPM 安装](https://npm.nodejs.cn/cli/v11/configuring-npm/install) 这里找到对应版本，本项目使用 nodejs 的版本为 `v22.11.0`，npm 的版本为 `11.1.0`，docsify 的版本为 `4.13.1`。

使用 `npm install docsify` 安装 docsify 包，安装完成后在终端执行以下指令：

```shell
docsify
```

如果出现以下内容说明安装完成：

```shell
Usage: docsify <init|serve> <path>

Commands:
  docsify init [path]      Creates new docs                         [aliases: i]
  docsify serve [path]     Run local server to preview site.        [aliases: s]
  docsify start <path>     Server for SSR
  docsify generate <path>  Docsify's generators                     [aliases: g]

Global Options
  --help, -h     Show help                                             [boolean]
  --version, -v  Show version number                                   [boolean]

Documentation:
  https://docsifyjs.github.io/docsify
  https://docsifyjs.github.io/docsify-cli

Development:
  https://github.com/docsifyjs/docsify-cli/blob/master/CONTRIBUTING.md


[ERROR] 0 arguments passed. Please specify a command
```

这时候 cd 到库的 `/docs` 目录下，使用指令 `docsify init` 初始化目录，然后执行 `docsify s` 在本地启动 web 服务，打开 `http://localhost:3000` 就可以看到文档了。

### 插入图片

建议按照 html 写法插入图片。其中用 `[]` 括起来的部分根据实际情况填写。需要在本地检查确定图片插入正常，大小合适。

```markdown
<img width=[250] alt="[any-title]" src="[/lab1/img/xxx.jpg]" style="margin:auto; display:flex;">
```

<mark>注意</mark>，正式部署前需要在所有的 `src` 关键字插入远程库名以保证部署后能正确引用图片。<br>
例如本代码仓库的名字叫做 `VLSI-Partition`，假设图片在本地部署时路径为 `/lab1/img/xxx.jpg`（注意是相对路径），那么改成以下形式：

```markdown
<img width=250 alt="any-title" src="/lab1/img/xxx.jpg" style="margin:auto; display:flex;">
改成以下形式
<img width=250 alt="any-title" src="/VLSI-Partition/lab1/img/xxx.jpg" style="margin:auto; display:flex;">
```

> 此时本地预览会提示图片路径错误，这是正常的。因此务必确保在本地预览时图片正确显示，然后再修改 src 关键字。<br>
> 建议新章节除了文件标题用一级标题外，其他位置不使用一级标题。

### 加入新章节

在 docs 目录下创建一个新的文件目录，在内部创建 `img` 目录用于存放图片，然后创建 markdown 文件。

形成的结构目录如下所示：

```shell
/docs
|--- ...
|--- lab1/
|    |--- img/
|    |--- lab1_problem.md
|    |--- lab1_problem_advanced.md
```

然后为新章节加入访问入口。打开 `/docs/_sidebar.md` 文件，找一个合适的位置添加入口，并添加连接。

### 修改主页

打开 `/docs/README.md` 文件，修改其中内容。

### 其他显示设置

比如修改侧边栏的标题层级、主题等，建议参考 [docsify 文档](https://docsify.js.org/#/)。

## 代码部分

检查对应作业文档。

## 目录结构

```
VLSI-Partition/
├── lab1/                  # 实验一代码（图划分算法）
│   ├── main.cpp
│   ├── solution.cpp/.h
│   ├── Graph.cpp/.h
│   ├── Node.cpp/.h
│   ├── Net.cpp/.h
│   ├── evaluate.cpp/.h
│   └── Makefile
├── docs/                  # 文档（docsify 部署）
│   ├── lab1/              # 实验一文档
│   ├── advanced/          # 进阶要求文档（划分）
│   ├── labFinal.md        # 大作业说明
│   └── ...
├── README.md
└── .gitignore
```
