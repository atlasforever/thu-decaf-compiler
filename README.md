# Decaf 编译器
使用 C++、ANTLR、LLVM 实现的 THU Decaf 编译器。

语言是清华大学2019版的 Decaf，一种 Java-like 语言。

```Decaf是一种强类型的、面向对象的、支持单继承和对象封装的语言。```

语言规范与官方版实现的说明文档：

* 官方repo：https://github.com/decaf-lang/decaf
* 语法规范：https://decaf-lang.gitbook.io/decaf-book/spec
* 官方实现指南：https://decaf-project.gitbook.io
* 测试集：https://github.com/decaf-lang/decaf-tests

## 开始
项目依赖：
* LLVM 13
* JDK 11

编译项目
```
make clean && make
```

运行测试
```shell
cd tests
./run-test 1    # 语法测试
./run-test 2    # 语义测试
./run-test 3    # 代码生成测试
```