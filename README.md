# Decaf 编译器
本编译器将 Decaf 代码编译为 LLVM IR。

THU Decaf 是来自清华大学2019版的Decaf语言，一种有类Java语法的程序语言。

```
Decaf是一种强类型的、面向对象的、支持单继承和对象封装的语言。
```

语言规范与官方版实现的说明文档：

* 官方repo：https://github.com/decaf-lang/decaf
* 语法规范：https://decaf-lang.gitbook.io/decaf-book/spec
* 官方实现指南：https://decaf-project.gitbook.io
* 测试集：https://github.com/decaf-lang/decaf-tests

## 开始
项目依赖：
* LLVM 12
* Clang 12
* JDK 11

编译项目：
```
make clean && make
```

测试：
```shell
cd tests
./run-test 1    # 语法测试
./run-test 2    # 语义测试
./run-test 3    # 代码生成测试
```


运行：

decaf 编译器不直接生成二进制可执行文件，只输出 LLVM IR。需要用`clang`生成可执行文件。
```shell
# 生成IR
./decaf tests/PA3/input/math.decaf > outir.ll

# 编译IR链接运行库，生成可执行文件 a.out
clang-13 ./outir.ll src/runtime/runtime.c -o a.out

./a.out
```
## 实现介绍
本项目使用 C++ 重新实现编译器，采用`ANTLR-4`作为 parser generator，并用`LLVM-13`库生成 LLVM IR。

### 语法分析
根据文法文件`src/parser/DecafLexer.g4`与`src/parser/DecafParser.g4`，antlr 将生成 C++ 格式的 lexer 和 parser。

借助 lexer 和 parser，`CommonLexer.cpp`会做检查字符串、数字常量等操作。

### 语义分析
参考：https://decaf-lang.gitbook.io/decaf-book/java-kuang-jia-fen-jie-duan-zhi-dao/pa2-yu-yi-fen-xi

流程由`SymbolChecker.cpp`和`TypeChecker.cpp`完成。

**SymbolChecker** 多趟遍历 AST，生成相应的作用域（符号表），并将类、类成员、局部变量等符号加入其中。顺便做部分基本的检查（如检查循环继承）。

**TypeChecker** 一遍遍历 AST，借助前面的符号表做详细的类型检查。具有类型猜测与错误恢复，以此进行尽可能多的类型检查。

### 代码生成
一次遍历 AST ，调用 LLVM 工具函数输出 LLVM IR。

针对每个类，分别创建两个对应的 LLVM StructType，表示类本身与其 vtable 的内存结构。

类的内存结构：
1. 指向 vtable 的指针
2. 所有基类的成员变量
3. 该类的成员变量

vtable 的内存结构：
1. 一个指向基类的 vtable 指针（用于 instanceOf ）
2. 一个指向类名字符串的指针（暂时无用？）
3. n 个指向所有函数（包括继承）的指针。如有函数重写，不追加指针，而是让被重写函数指针指向新函数。

虚函数调用：
编译时计算该函数在 vtable 中的偏移位置。运行时取出调用对象的 vtable，根据偏移获取函数指针。

内置函数：
运行时检查、打印等内置库函数位于`src/runtime/runtime.c`，如使用需链接进最终可执行文件。





