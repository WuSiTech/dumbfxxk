> **English version:** see the bottom of this README.

# 日⑨（dumbfuck / DF9）

**dumbfuck⑨**，缩写 **DF9**，中文名 **日⑨**。一门把 C 风格程序压成“看起来像乱码、但每个字符都有理由”的深奥语言。

和 Brainfuck 不同，DF9 的缩写理论上都助记：`DFI4main` 就是 **D**efine **F**unction **I**nt，**4** 个字符的名字 **main**。记住之后你还是看不懂，但至少可以跟别人解释你不是乱敲键盘。

它直接生成 **x86-64 AT&T 汇编**（Linux System V / Windows Win64），不经过 C。支持函数、数组、控制流、线程和字节 I/O；指针、浮点、字符串变量？编译器说：你想多了。


> 琪露诺小课堂：**⑨** = 琪露诺 = 笨蛋，**日⑨** 就是 dumbfuck 的中文意译（日笨蛋）

使用前请确保配置好gcc/g++的环境变量

---

## 💩 为什么要有 DF9？

* **极致压缩**：没有空格！没有分号！（但有些地方省了会咬人）。
* **视觉混沌**：你在写加密固件。
* **零 C 中间层**：编译器直接吐 `.s`；
* **一行艺术品**：红黑树可以压成一行。不是因为它优雅，而是因为它没有换行。
* **类型自由**：所有类型都是 32 位 int，从此没有类型焦虑，只有数值焦虑。
* **终于有 I/O 了**：`F3GET` / `F3PUT` / `F4EMIT`，能读源码、写汇编，顺便假装自举（`df9_df.df` 正在路上）。
* **社交价值**：把 DF9 代码发给同事，可以快速判断对方脾气。

---

## 🆕 这次新增了什么

* **函数调用能进表达式了**：`x&F3add(1,2)`、`F3PRT(F4fact(5))`、`F3add(F3add(1,2),3)` 都能跑，返回值终于有用了。
* **字节 I/O**：`F3GET` / `F3PUT` / `F4EMIT` / `F4EXIT`，外加一个还在实验中的 `F4PEEK`。
* **BOM 自动跳过**：`skip()` 现在会吃掉文件开头的 UTF-8 BOM，记事本不再暗算 `main`。
* **自举种子**：`df9_df.df`，一个用 DF9 写的代码生成器雏形。
* **Windows 图标构建**：`build_df9_exe.bat` 会把 `baka.ico` 嵌进 `df9.exe`。

---

## 🚀 快速开始

### 1. 构建编译器

需要 C++17 的 GCC / MinGW（源码用了 `bits/stdc++.h`，MSVC 默认编不了）。

Linux：

```bash
g++ -std=c++17 -O2 df9.cpp -o df9
```

Windows（MinGW-w64）：

```bat
g++ -std=c++17 -O2 -static -static-libstdc++ -static-libgcc df9.cpp -o df9.exe
```

Windows 下建议加 `-static` 系列参数，不然换台机器就缺 DLL。仓库里已经带了能用的 `df9.exe`，也可以直接用。想给编译器加琪露诺图标就运行 `build_df9_exe.bat`（`windres` + `baka.ico`）。

### 2. 写点 Dumbfuck

新建 `hello.df`，内容只有一行：

```text
DFI4main0,F3PRThello DF9;RET0'
```

翻译：

* `D`efine `F`unction `I`nt，名字长 `4`，名字 `main`，参数 `0` 个；
* `,` 开始多语句函数体；
* `F` + 长度 `3` + `PRT` = 打印；裸文本 `hello DF9` 一直到 `;` 结束；
* `RET0` 返回 0；
* `'` 结束函数体。

运行输出就是 `hello DF9`。

保存成 **UTF-8**。新版本的 `skip()` 会自动跳过文件开头的 UTF-8 BOM，BOM 不再吃 `main`。

### 3. 编译、链接、运行

Linux：

```bash
./df9 hello.df > hello.s
gcc -no-pie hello.s -o hello -lpthread -lm
./hello
```

Windows：

```bat
df9.exe hello.df > hello.s
gcc hello.s -o hello.exe -lpthread -lm
hello.exe
```

或者直接：

```text
compile.bat
Input File: hello.df
```

Windows 生存提示：PowerShell 5.1 的 `>` 会写出 UTF-16，GCC 看了会报 `invalid character`。用 `compile.bat`、`cmd /c`，或者显式转成 ASCII/UTF-8 无 BOM。

---

## 📖 九条戒律（The 9 Commandments）

### 1. 名字必须报长度

所有变量和函数声明都带名字长度。`DI3foo42` = `int foo = 42;`。函数调用也要：`F3foo(1,2)`。

长度写错不会报“长度错误”。它会默默把后面的 `(`、`;`、`'` 吃进名字，然后在更离谱的地方骂你。

> `Di1acc1` 声明的是 `a`，不是 `acc`。后面写 `acc`，编译器会说 `unknown variable: acc`。

### 2. 类型字母是滚木(不存在的)

`I`、`V`、`F`、`D`、`C`、`S`、`L`、`H`、`B`、`U` 只是让你想起 C。编译器只看它是不是字母，然后全部当 **32 位有符号 int**。

没有 float、没有 double、没有 char、没有字符串变量。`F` 不会变成浮点，只会让同事以为你会 C。

注意：变量声明不能用大写 `F`，因为 `DF` 会被认成函数定义；小写 `f` 可以。函数返回类型写 `F` 倒是可以（`DFF...`），但依然没意义。

### 3. 关键字是 F 词

格式：`F<长度><词>`。长度等于词的字符数。

`F2IF`、`F2EL`、`F1W`、`F2FR`、`F1B`、`F1C`、`F3RET`、`F2DL`、`F3PRT`、`F3JMP`。

**for 是 `F2FR`，不是 `F3FR`。** 这个错误值得单独写一行，因为很多人第一次都会写成 `F3FR`。

用户函数也走 F 词通道：`F3foo(...)` 调用 `foo`，`F10leftrotate(...)` 调用 `leftrotate`。

新增的 I/O 词单独列在下面：`F3GET`、`F4PEEK`、`F3PUT`、`F4EMIT`、`F4EXIT`。

### 4. 符号全换了

* `&` 是赋值：`x&1`
* `#` 是下标：`a#i`；赋值是 `a#i&1`
* `]` 是 `<`，`[` 是 `>`，`{` 是 `<=`，`}` 是 `>=`，`=` 是 `==`，`!` 是 `!=`

写 `x=1` 是比较相等，不是赋值；写 `a<b` 编译器不认识 `<`。C 程序员看到 `a]b` 会以为你在写正则。

另外：没有 `&&`、`||`、一元 `!`、位运算、移位。逻辑用 0/1 算术拼：

```text
a && b   →  (a!0)*(b!0)
a || b   →  ((a!0)+(b!0))!0
!a       →  (a=0)
```

### 5. 单引号结束一切

函数体、if、while、for 都以 `'` 结束。多语句函数体必须 `,` 开头。

```text
F2IF(x]5),F3PRT"small"'F2EL,F3PRT"big"'
```

嵌套几层就是 `''` / `'''` 的引号雨。别问，问就是 ⑨ 的天气。

空块是一个单独的 `'`；如果它同时结束外层块，就会看到 `''`。

### 6. 函数调用也是表达式

`F3foo(1,2);` 是语句；`x&F3foo(1,2)`、`F3PRT(F3foo(3,4))`、`F3foo(F3foo(1,2),3)` 也都合法。返回值在 `%eax` 里，现在能直接进表达式；函数可以递归，也可以嵌套调用。

```text
DFI3add2I1aI1b'RETa+b'
DFI4main0,DI1x0;x&F3add(2,3);F3PRT(x);F3PRT(F3add(4,5));F3PRT(F3add(F3add(1,2),3));RET0'
```

输出：

```text
5
9
6
```

递归也行：

```text
DFI4fact1I1n,F2IF(n{1),RET1'RET n*F4fact(n-1)'
DFI4main0,F3PRT(F4fact(5));RET0'
```

输出 `120`。

仍然没有参数个数检查：定义两个参数、调用一个也能编译，缺的那个是寄存器里的垃圾。参数上限是 Windows 4 个、Linux 6 个，超了才报错。

注意：表达式里看到 `F` 就按调用解析，所以变量名别用 `F` 开头（`Fact` 会死）。

### 7. 分号基本可选，但别真省

大多数语句靠文法自己结束；但裸 `PRT`、局部声明初始化、无值 `RET` 会咬人。

局部声明后面无脑加 `;`：

```text
DFI4main0,Di1x
F3PRT"hi";RET0'
```

上面不会打印 `hi`，而是把 `F3PRT...` 当成 `x` 的初始化表达式，然后报 `unknown variable: F3PRT`。

裸 `PRT` 后面也加 `;`，不然它会把后面的代码一起打印出来。

### 8. 只有 int 和一维数组

数组元素固定 4 字节。`DI3arr*10` 是 10 个 int 的全局数组，全 0；局部数组在栈上，未初始化。

没有指针、没有结构体、没有 `malloc`、没有字符串变量。数组不能传参，只能全局共享。越界不查，越到哪算哪。

`arr` 单独写等于 `arr[0]`。`arr#i+1` 是 `arr[i+1]`，不是元素加一；`arr#i]5` 是 `arr[i<5]`——给下标加括号，能救命。

### 9. 编译器直出汇编，bug 也是特性

单遍解析，直接吐 AT&T 汇编。没有 AST、没有 IR、没有优化——编译器觉得你的代码已经很抽象了。

所以有些行为请当成“风味”：

* for 里的 `continue` 跳过步进，不是 C 语义；
* 全局 `010` 是八进制（8），局部 `010` 是十进制（10）；
* 顶层不认识的行会被静默跳过；
* 错误只有 `pos`，没有行号，它相信你自己数；
* `F2DW` 是 do-while，编译器看到它会说：想得美。

---

## 🗺️翻译表

| 你想干的事 | C 里怎么写 | DF9 里怎么写 | 备注 |
| :--- | :--- | :--- | :--- |
| 全局变量 | `int foo = 42;` | `DI3foo42` | D + I + 长度 3 + foo + 42 |
| 全局数组 | `int a[10];` | `DI1a*10` | 全 0，放 `.bss` |
| 局部变量 | `int x = 5;` | `Di1x5` | 局部初始化不用 `&` |
| 函数 | `void f(){ return; }` | `DFV1f'RET'` | 多语句体要 `,` 开头 |
| 赋值 | `x = a + b;` | `x&a+b` | `&` 是赋值 |
| 数组赋值 | `a[i] = x;` | `a#i&x` | 下标是 `#` |
| if | `if (x < 5) { ... }` | `F2IF(x]5),...'` | 条件后要逗号 |
| else | `else { ... }` | `F2EL,...'` | 必须紧跟 then 的闭引号 |
| while | `while (x < 5) { ... }` | `F1W(x]5),...'` | 先判断 |
| for | `for (i=0; i<5; i++)` | `F2FRi&0~i]5~i&i+1,...'` | **F2FR**，不是 F3FR |
| break / continue | `break;` / `continue;` | `F1B` / `F1C` | 只能在循环里 |
| return | `return x;` | `F3RETx` 或 `RETx` | 返回值现在能进表达式 |
| 打印整数 | `printf("%d\n", x);` | `F3PRT(x)` | 自动换行 |
| 打印字符串 | `printf("hi\n");` | `F3PRT"hi"` 或 `F3PRThi` | `hi` 裸文本到 `;` 结束 |
| 调用 | `foo(1, 2);` | `F3foo(1,2)` | 可当语句，也可进表达式 |
| 标签 / 跳转 | `label:` / `goto label;` | `@2lp` / `F3JMPlp` | 定义带长度，跳转不带 |
| 延时 | `usleep(1000000);` | `F2DL60` | 单位约 1/60 秒 |
| 线程 | `pthread_create(...)` | `^F3foo()` | 相邻 `^` 一起并发 |
| 读入字节 | `getchar()` | `c&F3GET()` | EOF 返回 -1 |
| 输出字节 | `putchar(c)` | `F3PUT(c)` | 必须括号；不换行 |
| 输出字符串 | `puts("hi")` | `F4EMIT("hi")` | 必须括号；自动换行 |
| 退出 | `exit(0);` | `F4EXIT` | 固定退出码 0，无参数 |

> 表里的 `...` 不是语法，是“这里还有代码”的意思。真正的源码里不要写省略号，除非你想看报错。

---

## 🔌 新 I/O 原语

`df9.cpp` 新加了一组直接读写字节的内建词，自举种子就靠它们活着：

| 写法 | 作用 | 备注 |
| --- | --- | --- |
| `F3GET()` / `F3GET` | 从 stdin 读一个字节，0..255，EOF 返回 -1 | 括号可省；可进表达式 |
| `F4PEEK()` / `F4PEEK` | 偷看下一个字节 | 实验性：当前版本会卡住/崩溃（`ungetc` 的 `FILE*` 参数不对），先别用 |
| `F3PUT(expr)` | 输出一个字节，不换行 | 必须括号 |
| `F4EMIT("text")` / `F4EMIT(text)` | `puts` 输出 + 换行 | 必须括号；引号或裸文本都行 |
| `F4EXIT` / `F4EXIT()` | 退出，状态 0 | 无参数；`F4EXIT(42)` 会编译报错 |

它们不是普通函数，是 `parseF` 直接处理的内建词；生成代码会调用 `__df_get` / `__df_put` / `__df_emit` / `__df_peek` 这几个小运行时。

回显示例（喂什么吐什么，EOF 退出）：

```text
DFI4main0,DI1c0;F1W(1),c&F3GET();F2IF(c=-1),F1B'F3PUT(c)'F4EXIT'
```

`F3GET()` 在 EOF 返回 -1，所以 `c=-1` 可以当循环结束条件。

```text
DFI4main0,F4EMIT("hello from EMIT");F3PUT(65);F3PUT(10);F4EXIT
```

输出两行：`hello from EMIT`，然后 `A`。`F4EMIT` 自动换行，`F3PUT` 不会。

> ⚠️ `F4PEEK` 目前是半成品：生成的 `ungetc` 没拿到有效的 `FILE*`，我这边测试会卡住。把它当实验接口。

---

## 🥾 自举种子：df9_df.df

`df9_df.df` 是一个用 DF9 写的“自举种子”：它把源码读进 `SRC*65536`，用 `F3GET` 读 stdin，用 `F4EMIT` / `F3PUT` 往 stdout 吐汇编。它不是完整编译器——目前最稳定的子集是 `RET...`，带 `PRT` 的路径还在施工——但它已经是自举的第一步，也顺便把新 I/O 和表达式调用都用上了。

<details>
<summary>点开自举种子（一行 DF9）</summary>

```text
DI3SRC*65536;DI1P0;DI1L0;DI1C0;DFV4read0,DI1c0;F1W(1),c&F3GET();F2IF(c=-1),F1B'SRC#P&c;P&P+1'L&P;P&0'DFV4skip0,F1W(1),F2IF((SRC#P)=32),P&P+1'F2EL,F2IF((SRC#P)=10),P&P+1'F2EL,F1B''''DFI6getnum0,DI1n0;F4skip();F1W(1),F2IF((SRC#P){58),F2IF((SRC#P)}47),n&n*10+((SRC#P)-48)'P&P+1'F2EL,F1B'RET n''DFV5genas0,F4read();F4EMIT(".text");F4EMIT(".globl main");F4EMIT("main:");F4EMIT(" push %rbp");F4EMIT(" mov %rsp,%rbp");F1W(1),F4skip();F2IF((SRC#P)=0),F1B'C&SRC#P;F2IF(C=70),P&P+1;L&F6getnum();P&P+L;F2IF((SRC#P)=80),P&P+3;F2IF((SRC#P)=34),P&P+1'F1W(1),F2IF((SRC#P)=34),F1B'F2IF((SRC#P)=0),F1B'F3PUT(SRC#P);P&P+1'F2IF((SRC#P)=34),P&P+1'F4EMIT("");''F2IF(C=82),P&P+3;F4EMIT(" xor %eax,%eax");F4EMIT(" leave");F4EMIT(" ret");F4EXIT'P&P+1''DFI4main0,F5genas();F4EXIT'
```

</details>

构建并运行：

```text
df9.exe df9_df.df > df9_df.s
gcc df9_df.s -o df9_df.exe -lpthread -lm
df9_df.exe < seed_input.df > out.s
```

最小输入 `seed_input.df`：

```text
RET0'
```

生成的 `out.s`：

```text
.text
.globl main
main:
 push %rbp
 mov %rsp,%rbp
 xor %eax,%eax
 leave
 ret
```

`RET` 路径是通的；`FPRT"..."` 目前还不行（会把文本直接写进 `.text`，GCC 不认）。完整自举还在路上。

---

## 🎁补丁

* **`Di1acc1` 不是 `acc`。** 在顶层，名字长度是 1，所以它声明的是 `a`；`cc1` 被无情抛弃。
* **局部声明不写分号，下一句会被吃掉。** 因为换行只是空白，不是语句结束。
* **else 前不要加分号。** `...'F2EL,...` 才是对的；`...';F2EL,...` 会让 `F2EL` 变成函数调用。
* **for 的 `continue` 跳过步进。** 你会看到 `0 1 3 4` 而不是 `0 1 4`，然后开始怀疑人生。
* **全局 `010` 是 8，局部 `010` 是 10。** 全局初值进 GAS 的 `.long`，前导 0 按八进制；局部/表达式走 C++ 的 `stoi`，十进制。
* **数组下标会贪婪。** `arr#i+1` 是 `arr[i+1]`；`arr#i]5` 是 `arr[i<5]`。括号是救生圈。
* **顶层标签是死的。** `@` 在顶层会被解析，但不会输出；函数里跳过去就是 undefined。`.L` 标签还是全文件共享，两个函数用同名标签会撞车。
* **线程里别碰局部变量。** 线程体被复制成独立函数，但没有为局部变量分配栈空间；`%rbp` 不会替你圆场。
* **`F` 开头的变量名在表达式里会被当成函数调用。** 定义 `Fact` 也没用；`F3PRT(Fact)` 会去找 `(`，找不到就报错。变量别用 `F` 开头。

> 🎁 额外彩蛋：`F2DW` 是 do-while。编译器看到它会说：想得美。

---

## 🏆 终极测试：一行红黑树

官方压力测试：五个全局数组 + 一堆 `F2EL`，实现插入 + 中序遍历。

下面这行真的能跑，输出还是有序的

<details>
<summary>点开受诅咒的一行（真的能跑）</summary>

```text
DI3KEY*21;DI2LC*21;DI2RC*21;DI2PA*21;DI2CO*21;DI4ROOT0;DI4NEXT0;DFV6insert1I1k,DI1z0;DI1y0;DI1x0;z&NEXT;NEXT&NEXT+1;KEY#z&k;LC#z&0;RC#z&0;PA#z&0;CO#z&0;y&0;x&ROOT;F1W(x!0),y&x;F2IF(k](KEY#x)),x&LC#x'F2EL,x&RC#x''PA#z&y;F2IF(y=0),ROOT&z'F2EL,F2IF(k](KEY#y)),LC#y&z'F2EL,RC#y&z''CO#z&0;F5fixup(z)'DFV5fixup1I1z,DI1y0;F1W((PA#z)!0),F2IF(((CO#PA#z)!0)),F1B'F2IF((PA#z)=(LC#(PA#PA#z))),y&RC#(PA#PA#z);F2IF((CO#y)=0),CO#PA#z&1;CO#y&1;CO#(PA#PA#z)&0;z&PA#PA#z'F2EL,F2IF(z=(RC#(PA#z))),z&PA#z;F10leftrotate(z)'CO#PA#z&1;CO#(PA#PA#z)&0;F11rightrotate(PA#PA#z)''F2EL,y&LC#(PA#PA#z);F2IF((CO#y)=0),CO#PA#z&1;CO#y&1;CO#(PA#PA#z)&0;z&PA#PA#z'F2EL,F2IF(z=(LC#(PA#z))),z&PA#z;F11rightrotate(z)'CO#PA#z&1;CO#(PA#PA#z)&0;F10leftrotate(PA#PA#z)'''CO#ROOT&1'DFV10leftrotate1I1x,DI1y0;y&RC#x;RC#x&LC#y;F2IF((LC#y)!0),PA#LC#y&x'PA#y&PA#x;F2IF((PA#x)=0),ROOT&y'F2EL,F2IF((LC#(PA#x))=x),LC#(PA#x)&y'F2EL,RC#(PA#x)&y''LC#y&x;PA#x&y'DFV11rightrotate1I1x,DI1y0;y&LC#x;LC#x&RC#y;F2IF((RC#y)!0),PA#RC#y&x'PA#y&PA#x;F2IF((PA#x)=0),ROOT&y'F2EL,F2IF((RC#(PA#x))=x),RC#(PA#x)&y'F2EL,LC#(PA#x)&y''RC#y&x;PA#x&y'DFV7inorder1I1x,F2IF(x!0),F7inorder(LC#x);F3PRT(KEY#x);F7inorder(RC#x)''DFI4main0,NEXT&1;F6insert(10);F6insert(6);F6insert(18);F6insert(3);F6insert(8);F6insert(11);F6insert(22);F6insert(2);F6insert(7);F6insert(26);F7inorder(ROOT);RET0'
```

</details>

**Expected Output:**

```text
2
3
6
7
8
10
11
18
22
26
```

如果输出不是这个，先检查你是不是复制时把某个 `'` 弄丢了。红黑树不会原谅你，编译器也不会。

---

## ⚙️ 编译器架构

* **语言**：C++17。
* **解析**：递归下降，AST-less，单遍；边解析边拼汇编。
* **表达式**：函数调用也是 primary，所以能当语句，也能进算术/条件/参数。
* **Linux**：System V ABI，最多 6 个参数寄存器。
* **Windows**：Win64 ABI，最多 4 个参数寄存器；
* **内存**：数组元素固定 4 字节；全局进 `.bss` / `.data`；局部在 `rbp-offset`。
* **I/O 运行时**：GET/PUT/EMIT/PEEK 生成 `__df_get` / `__df_put` / `__df_emit` / `__df_peek`，底下是 `getchar` / `putchar` / `puts` / `ungetc`。
* **BOM**：`skip()` 在文件开头遇到 UTF-8 BOM 会自动跳过。
* **线程**：`^` 生成 `pthread_create` / `pthread_join`，每个线程是一个隐式函数。
* **优化**：没有。编译器觉得你的代码已经够抽象了。
* **报错**：只有 `pos`，没有行号；它相信你自己数。

---

## ⚠️ 免责声明

作者不对以下情况负责：脑损伤、眼出血、误提交、同事报警、代码评审被打、以及你试图用 DF9 写 Web 后端。

如果你选择用 dumbfuck⑨ 写生产代码，你活该。

本文中的完整程序在仓库自带 `df9.exe` + MinGW-w64 gcc 11.5 上实际编译运行过；Linux 分支按源码推导，理论上也能跑。跑不了别找我，找 ABI。

**License:** MIT (Make It Torturous)。

---

# 日⑨ (dumbfuck / DF9) — English Version

> *"Every byte counts. No whitespace spared. Sanity not included."*

**dumbfuck⑨**, abbreviated **DF9**, Chinese name **日⑨**. An esoteric language that compresses C-style programs into something that "looks like gibberish, but every character has a reason."

Unlike Brainfuck, DF9's abbreviations are at least theoretically mnemonic: `DFI4main` is **D**efine **F**unction **I**nt, **4**-character name **main**. After memorizing that, you still can't read it—but at least you can explain to people that you're not just mashing the keyboard.

It compiles directly to **x86-64 AT&T assembly** (Linux System V / Windows Win64), with no C in between. It supports functions, arrays, control flow, threads, and byte-level I/O. Pointers, floats, string variables? The compiler says: you wish.

> Cirno's Little Lesson: **⑨** = Cirno = idiot. **日⑨** is the Chinese rendering of dumbfuck ("fuck" + "idiot").

Make sure `gcc` / `g++` are installed and on your PATH before use.

---

## 💩 Why Does DF9 Exist?

* **Extreme compression**: no spaces! no semicolons! (Though omitting some of them will bite you.)
* **Visual chaos**: you are writing encrypted firmware.
* **Zero C intermediate layer**: the compiler spits out `.s` directly.
* **One-line art**: an entire red-black tree fits on one line. Not because it's elegant, but because it has no line breaks.
* **Type freedom**: every type is a 32-bit int. No type anxiety—only value anxiety.
* **I/O at last**: `F3GET` / `F3PUT` / `F4EMIT`, so it can read source, write assembly, and pretend to self-host (`df9_df.df` is on the way).
* **Social value**: send DF9 code to a coworker and quickly gauge their temper.

---

## 🆕 What's New

* **Function calls work in expressions now**: `x&F3add(1,2)`, `F3PRT(F4fact(5))`, and `F3add(F3add(1,2),3)` all run; return values are finally useful.
* **Byte I/O**: `F3GET` / `F3PUT` / `F4EMIT` / `F4EXIT`, plus an experimental `F4PEEK`.
* **Automatic BOM skip**: `skip()` now eats a UTF-8 BOM at the start of the file, so Notepad can't ambush `main`.
* **Self-hosting seed**: `df9_df.df`, a DF9-written code generator in its infancy.
* **Windows icon build**: `build_df9_exe.bat` bakes `baka.ico` into `df9.exe`.

---

## 🚀 Quick Start

### 1. Build the Compiler

Requires GCC / MinGW with C++17 (the source uses `bits/stdc++.h`; MSVC can't compile it out of the box).

Linux:

```bash
g++ -std=c++17 -O2 df9.cpp -o df9
```

Windows (MinGW-w64):

```bat
g++ -std=c++17 -O2 -static -static-libstdc++ -static-libgcc df9.cpp -o df9.exe
```

On Windows, the `-static` flags are recommended; otherwise the binary will be missing DLLs on another machine. The repo already includes a working `df9.exe`, so you can also use that directly. If you want the Cirno icon baked in, run `build_df9_exe.bat` (`windres` + `baka.ico`).

### 2. Write Some Dumbfuck

Create `hello.df` with a single line:

```text
DFI4main0,F3PRThello DF9;RET0'
```

Translation:

* `D`efine `F`unction `I`nt, name length `4`, name `main`, `0` arguments;
* `,` starts a multi-statement function body;
* `F` + length `3` + `PRT` = print; raw text `hello DF9` runs until `;`;
* `RET0` returns 0;
* `'` closes the function body.

Running it prints `hello DF9`.

Save it as **UTF-8**. The new `skip()` automatically eats a UTF-8 BOM at the start of the file, so a BOM no longer eats `main`.

### 3. Compile, Link, Run

Linux:

```bash
./df9 hello.df > hello.s
gcc -no-pie hello.s -o hello -lpthread -lm
./hello
```

Windows:

```bat
df9.exe hello.df > hello.s
gcc hello.s -o hello.exe -lpthread -lm
hello.exe
```

Or just:

```text
compile.bat
Input File: hello.df
```

Windows survival tip: PowerShell 5.1's `>` writes UTF-16, and GCC will complain about `invalid character`. Use `compile.bat`, `cmd /c`, or explicitly save as ASCII/UTF-8 without BOM.

---

## 📖 The 9 Commandments

### 1. Names Must Carry Their Length

Every variable and function declaration carries the name's length. `DI3foo42` = `int foo = 42;`. Function calls need it too: `F3foo(1,2)`.

Get the length wrong and you won't get a "length error". It will quietly swallow the following `(`, `;`, `'` into the name, then yell at you somewhere far more confusing.

> `Di1acc1` declares `a`, not `acc`. Write `acc` later and the compiler says `unknown variable: acc`.

### 2. Type Letters Are Nonexistent

`I`, `V`, `F`, `D`, `C`, `S`, `L`, `H`, `B`, `U` are just there to remind you of C. The compiler only checks that it's a letter, then treats everything as a **32-bit signed int**.

No float, no double, no char, no string variables. `F` will not become a float; it will only make your coworkers think you know C.

Note: variable declarations cannot use uppercase `F`, because `DF` is parsed as a function definition. Lowercase `f` is fine. A function return type of `F` is allowed (`DFF...`), but still means nothing.

### 3. Keywords Are F-Words

Format: `F<length><word>`. The length equals the number of characters in the word.

`F2IF`, `F2EL`, `F1W`, `F2FR`, `F1B`, `F1C`, `F3RET`, `F2DL`, `F3PRT`, `F3JMP`.

**for is `F2FR`, not `F3FR`.** This mistake deserves its own line, because almost everyone writes `F3FR` the first time.

User functions also go through the F-word channel: `F3foo(...)` calls `foo`, and `F10leftrotate(...)` calls `leftrotate`.

The new I/O words get their own section below: `F3GET`, `F4PEEK`, `F3PUT`, `F4EMIT`, `F4EXIT`.

### 4. All the Symbols Got Swapped

* `&` is assignment: `x&1`
* `#` is a subscript: `a#i`; assignment is `a#i&1`
* `]` is `<`, `[` is `>`, `{` is `<=`, `}` is `>=`, `=` is `==`, `!` is `!=`

Writing `x=1` is equality, not assignment. Writing `a<b` leaves the compiler wondering what `<` is. A C programmer seeing `a]b` will think you're writing regex.

Also: no `&&`, no `||`, no unary `!`, no bitwise operators, no shifts. Build logic out of 0/1 arithmetic:

```text
a && b   →  (a!0)*(b!0)
a || b   →  ((a!0)+(b!0))!0
!a       →  (a=0)
```

### 5. A Single Quote Ends Everything

Function bodies, if, while, and for all end with `'`. Multi-statement function bodies must start with `,`.

```text
F2IF(x]5),F3PRT"small"'F2EL,F3PRT"big"'
```

Nest a few and you get `''` / `'''` quote rain. Don't ask; it's just ⑨ weather.

An empty block is a single `'`; if it also closes the outer block, you'll see `''`.

### 6. Function Calls Are Expressions Too

`F3foo(1,2);` is a statement; `x&F3foo(1,2)`, `F3PRT(F3foo(3,4))`, and `F3foo(F3foo(1,2),3)` are all legal. The return value lives in `%eax` and can now enter expressions directly. Functions can recurse and calls can nest.

```text
DFI3add2I1aI1b'RETa+b'
DFI4main0,DI1x0;x&F3add(2,3);F3PRT(x);F3PRT(F3add(4,5));F3PRT(F3add(F3add(1,2),3));RET0'
```

Output:

```text
5
9
6
```

Recursion works too:

```text
DFI4fact1I1n,F2IF(n{1),RET1'RET n*F4fact(n-1)'
DFI4main0,F3PRT(F4fact(5));RET0'
```

That prints `120`.

There is still no arity check: define two parameters, call with one, and it compiles; the missing one is garbage from a register. The hard cap is 4 arguments on Windows and 6 on Linux; exceed it and you get an error.

Note: an `F` in an expression is parsed as a call, so don't start variable names with `F` (`Fact` is dead).

### 7. Semicolons Are Basically Optional—Don't Push It

Most statements end themselves grammatically, but bare `PRT`, local declaration initializers, and valueless `RET` will bite you.

Put a semicolon after every local declaration, no thinking required:

```text
DFI4main0,Di1x
F3PRT"hi";RET0'
```

The code above does not print `hi`; it treats `F3PRT...` as `x`'s initializer and reports `unknown variable: F3PRT`.

Also put a `;` after a bare `PRT`, or it will print the rest of your code too.

### 8. Only int and One-Dimensional Arrays

Array elements are always 4 bytes. `DI3arr*10` is a global array of 10 ints, all zero; local arrays live on the stack and are uninitialized.

No pointers, no structs, no `malloc`, no string variables. Arrays cannot be passed as parameters—share them globally. No bounds checks; wherever it lands is where it lands.

`arr` alone equals `arr[0]`. `arr#i+1` is `arr[i+1]`, not element-plus-one; `arr#i]5` is `arr[i<5]`—parenthesize your indices; it saves lives.

### 9. The Compiler Emits Assembly Directly; Bugs Are Features

Single-pass parsing, straight to AT&T assembly. No AST, no IR, no optimization—the compiler thinks your code is already abstract enough.

So treat some behaviors as "flavor":

* `continue` inside a for loop skips the step, because that's not C semantics;
* a global `010` is octal (8), while a local `010` is decimal (10);
* unrecognized top-level lines are silently skipped;
* errors give only a `pos`, no line number; the compiler trusts you to count;
* `F2DW` is do-while, and the compiler responds with: nice try.

---

## 🗺️ Translation Table

| What you want | In C | In DF9 | Notes |
| :--- | :--- | :--- | :--- |
| Global variable | `int foo = 42;` | `DI3foo42` | D + I + length 3 + foo + 42 |
| Global array | `int a[10];` | `DI1a*10` | all zeros, in `.bss` |
| Local variable | `int x = 5;` | `Di1x5` | local init doesn't use `&` |
| Function | `void f(){ return; }` | `DFV1f'RET'` | multi-statement body needs `,` |
| Assignment | `x = a + b;` | `x&a+b` | `&` is assignment |
| Array assignment | `a[i] = x;` | `a#i&x` | `#` is subscript |
| if | `if (x < 5) { ... }` | `F2IF(x]5),...'` | comma after the condition |
| else | `else { ... }` | `F2EL,...'` | must hug then's closing quote |
| while | `while (x < 5) { ... }` | `F1W(x]5),...'` | check first |
| for | `for (i=0; i<5; i++)` | `F2FRi&0~i]5~i&i+1,...'` | **F2FR**, not F3FR |
| break / continue | `break;` / `continue;` | `F1B` / `F1C` | loops only |
| return | `return x;` | `F3RETx` or `RETx` | return value now usable in expressions |
| Print integer | `printf("%d\n", x);` | `F3PRT(x)` | auto newline |
| Print string | `printf("hi\n");` | `F3PRT"hi"` or `F3PRThi` | raw `hi` ends at `;` |
| Call | `foo(1, 2);` | `F3foo(1,2)` | statement or expression |
| Label / goto | `label:` / `goto label;` | `@2lp` / `F3JMPlp` | definition has length; jump doesn't |
| Delay | `usleep(1000000);` | `F2DL60` | unit is about 1/60 second |
| Thread | `pthread_create(...)` | `^F3foo()` | adjacent `^` run concurrently |
| Read byte | `getchar()` | `c&F3GET()` | returns -1 at EOF |
| Write byte | `putchar(c)` | `F3PUT(c)` | parentheses required; no newline |
| Write string | `puts("hi")` | `F4EMIT("hi")` | parentheses required; auto newline |
| Exit | `exit(0);` | `F4EXIT` | always status 0; no arguments |

> The `...` in the table is not syntax. It means "there is more code here". Do not write ellipses in real source unless you enjoy error messages.

---

## 🔌 New Low-Level I/O

`df9.cpp` now has a set of byte-level I/O builtins. The self-hosting seed lives on them:

| Form | What it does | Notes |
| --- | --- | --- |
| `F3GET()` / `F3GET` | Read one byte from stdin, 0..255, -1 at EOF | parentheses optional; usable in expressions |
| `F4PEEK()` / `F4PEEK` | Peek at the next byte | Experimental: current version hangs/crashes (wrong `FILE*` for `ungetc`); don't use |
| `F3PUT(expr)` | Write one byte, no newline | parentheses required |
| `F4EMIT("text")` / `F4EMIT(text)` | `puts` output + newline | parentheses required; quoted or raw |
| `F4EXIT` / `F4EXIT()` | Exit with status 0 | no arguments; `F4EXIT(42)` is a compile error |

They are not ordinary functions; `parseF` handles them and emits calls to the tiny runtime helpers `__df_get` / `__df_put` / `__df_emit` / `__df_peek`.

Echo example (whatever you feed it comes back, EOF exits):

```text
DFI4main0,DI1c0;F1W(1),c&F3GET();F2IF(c=-1),F1B'F3PUT(c)'F4EXIT'
```

`F3GET()` returns -1 at EOF, so `c=-1` works as the loop terminator.

```text
DFI4main0,F4EMIT("hello from EMIT");F3PUT(65);F3PUT(10);F4EXIT
```

Two lines: `hello from EMIT`, then `A`. `F4EMIT` adds a newline; `F3PUT` doesn't.

> ⚠️ `F4PEEK` is half-finished: the generated `ungetc` call doesn't get a valid `FILE*`, and it hangs in my test. Treat it as experimental.

---

## 🥾 Self-Hosting Seed: df9_df.df

`df9_df.df` is a DF9-written "self-hosting seed": it reads source into `SRC*65536`, uses `F3GET` for stdin and `F4EMIT` / `F3PUT` for assembly on stdout. It is not a full compiler—the most stable subset is `RET...`, and the `PRT` path is still under construction—but it is the first step toward self-hosting, and it exercises the new I/O and expression calls.

<details>
<summary>Open the self-hosting seed (one line of DF9)</summary>

```text
DI3SRC*65536;DI1P0;DI1L0;DI1C0;DFV4read0,DI1c0;F1W(1),c&F3GET();F2IF(c=-1),F1B'SRC#P&c;P&P+1'L&P;P&0'DFV4skip0,F1W(1),F2IF((SRC#P)=32),P&P+1'F2EL,F2IF((SRC#P)=10),P&P+1'F2EL,F1B''''DFI6getnum0,DI1n0;F4skip();F1W(1),F2IF((SRC#P){58),F2IF((SRC#P)}47),n&n*10+((SRC#P)-48)'P&P+1'F2EL,F1B'RET n''DFV5genas0,F4read();F4EMIT(".text");F4EMIT(".globl main");F4EMIT("main:");F4EMIT(" push %rbp");F4EMIT(" mov %rsp,%rbp");F1W(1),F4skip();F2IF((SRC#P)=0),F1B'C&SRC#P;F2IF(C=70),P&P+1;L&F6getnum();P&P+L;F2IF((SRC#P)=80),P&P+3;F2IF((SRC#P)=34),P&P+1'F1W(1),F2IF((SRC#P)=34),F1B'F2IF((SRC#P)=0),F1B'F3PUT(SRC#P);P&P+1'F2IF((SRC#P)=34),P&P+1'F4EMIT("");''F2IF(C=82),P&P+3;F4EMIT(" xor %eax,%eax");F4EMIT(" leave");F4EMIT(" ret");F4EXIT'P&P+1''DFI4main0,F5genas();F4EXIT'
```

</details>

Build and run:

```text
df9.exe df9_df.df > df9_df.s
gcc df9_df.s -o df9_df.exe -lpthread -lm
df9_df.exe < seed_input.df > out.s
```

Minimal `seed_input.df`:

```text
RET0'
```

Generated `out.s`:

```text
.text
.globl main
main:
 push %rbp
 mov %rsp,%rbp
 xor %eax,%eax
 leave
 ret
```

The `RET` path works; `FPRT"..."` still does not (it writes text straight into `.text`, and GCC rejects it). Full self-hosting is still on the way.

---

## 🎁 Patches

* **`Di1acc1` is not `acc`.** At top level, the name length is 1, so it declares `a`; `cc1` is abandoned without mercy.
* **A local declaration without a semicolon eats the next statement.** A newline is whitespace, not a terminator.
* **Don't put a semicolon before else.** `...'F2EL,...` is correct; `...';F2EL,...` turns `F2EL` into a function call.
* **`continue` in a for loop skips the step.** You'll see `0 1 3 4` instead of `0 1 4`, then start questioning your life choices.
* **Global `010` is 8; local `010` is 10.** Global initializers go through GAS `.long` (octal); locals and expressions go through C++ `stoi` (decimal).
* **Array subscripts are greedy.** `arr#i+1` is `arr[i+1]`; `arr#i]5` is `arr[i<5]`. Parentheses are life preservers.
* **Top-level labels are dead.** `@` is parsed at top level but never emitted; jumping there is undefined. `.L` labels are file-wide; two functions with the same label collide.
* **Don't touch locals inside threads.** The thread body is copied into a separate function but gets no stack space for locals; `%rbp` will not save you.
* **A variable name starting with `F` is parsed as a call in expressions.** Defining `Fact` won't save you; `F3PRT(Fact)` looks for a `(` and fails. Don't start variable names with `F`.

> 🎁 Extra egg: `F2DW` is do-while. The compiler sees it and says: nice try.

---

## 🏆 The Ultimate Test: A One-Line Red-Black Tree

The official stress test: five global arrays + a pile of `F2EL`, implementing insertion + inorder traversal.

This line actually runs, and the output is sorted.

<details>
<summary>Open the cursed one-liner (it really runs)</summary>

```text
DI3KEY*21;DI2LC*21;DI2RC*21;DI2PA*21;DI2CO*21;DI4ROOT0;DI4NEXT0;DFV6insert1I1k,DI1z0;DI1y0;DI1x0;z&NEXT;NEXT&NEXT+1;KEY#z&k;LC#z&0;RC#z&0;PA#z&0;CO#z&0;y&0;x&ROOT;F1W(x!0),y&x;F2IF(k](KEY#x)),x&LC#x'F2EL,x&RC#x''PA#z&y;F2IF(y=0),ROOT&z'F2EL,F2IF(k](KEY#y)),LC#y&z'F2EL,RC#y&z''CO#z&0;F5fixup(z)'DFV5fixup1I1z,DI1y0;F1W((PA#z)!0),F2IF(((CO#PA#z)!0)),F1B'F2IF((PA#z)=(LC#(PA#PA#z))),y&RC#(PA#PA#z);F2IF((CO#y)=0),CO#PA#z&1;CO#y&1;CO#(PA#PA#z)&0;z&PA#PA#z'F2EL,F2IF(z=(RC#(PA#z))),z&PA#z;F10leftrotate(z)'CO#PA#z&1;CO#(PA#PA#z)&0;F11rightrotate(PA#PA#z)''F2EL,y&LC#(PA#PA#z);F2IF((CO#y)=0),CO#PA#z&1;CO#y&1;CO#(PA#PA#z)&0;z&PA#PA#z'F2EL,F2IF(z=(LC#(PA#z))),z&PA#z;F11rightrotate(z)'CO#PA#z&1;CO#(PA#PA#z)&0;F10leftrotate(PA#PA#z)'''CO#ROOT&1'DFV10leftrotate1I1x,DI1y0;y&RC#x;RC#x&LC#y;F2IF((LC#y)!0),PA#LC#y&x'PA#y&PA#x;F2IF((PA#x)=0),ROOT&y'F2EL,F2IF((LC#(PA#x))=x),LC#(PA#x)&y'F2EL,RC#(PA#x)&y''LC#y&x;PA#x&y'DFV11rightrotate1I1x,DI1y0;y&LC#x;LC#x&RC#y;F2IF((RC#y)!0),PA#RC#y&x'PA#y&PA#x;F2IF((PA#x)=0),ROOT&y'F2EL,F2IF((RC#(PA#x))=x),RC#(PA#x)&y'F2EL,LC#(PA#x)&y''RC#y&x;PA#x&y'DFV7inorder1I1x,F2IF(x!0),F7inorder(LC#x);F3PRT(KEY#x);F7inorder(RC#x)''DFI4main0,NEXT&1;F6insert(10);F6insert(6);F6insert(18);F6insert(3);F6insert(8);F6insert(11);F6insert(22);F6insert(2);F6insert(7);F6insert(26);F7inorder(ROOT);RET0'
```

</details>

**Expected Output:**

```text
2
3
6
7
8
10
11
18
22
26
```

If the output isn't this, first check whether you lost a `'` while copying. The red-black tree will not forgive you, and neither will the compiler.

---

## ⚙️ Compiler Architecture

* **Language**: C++17.
* **Parsing**: recursive descent, AST-less, single-pass; assembly is stitched together while parsing.
* **Expressions**: function calls are primaries too, so they work as statements and inside arithmetic, conditions, and arguments.
* **Linux**: System V ABI, at most 6 argument registers.
* **Windows**: Win64 ABI, at most 4 argument registers.
* **Memory**: array elements are 4 bytes; globals go to `.bss` / `.data`; locals live at `rbp-offset`.
* **I/O runtime**: GET/PUT/EMIT/PEEK emit `__df_get` / `__df_put` / `__df_emit` / `__df_peek`, built on `getchar` / `putchar` / `puts` / `ungetc`.
* **BOM**: `skip()` automatically skips a UTF-8 BOM at the start of the file.
* **Threads**: `^` emits `pthread_create` / `pthread_join`; each thread is an implicit function.
* **Optimization**: none. The compiler thinks your code is already abstract enough.
* **Errors**: only `pos`, no line numbers; it trusts you to count.

---

## ⚠️ Disclaimer

The author is not responsible for: brain damage, eye bleeding, accidental commits, coworkers calling the police, getting beaten up in code review, or your attempt to write a web backend in DF9.

If you choose to write production code in dumbfuck⑨, you deserve whatever happens.

The complete programs in this README were actually compiled and run with the bundled `df9.exe` + MinGW-w64 gcc 11.5; the Linux branch is derived from the source and should theoretically work. If it doesn't, don't blame me—blame the ABI.

**License:** MIT (Make It Torturous).

