# CuteLog

基于现代 C++ 实现的日志系统

## 特点

- 功能简单
- 使用方便：debug、info、warn、error、fatal 宏
- 模板 + 宏
- 彩色输出
- 多线程安全
- 输出到文件

## 需求

1. 日志存储：文本文件
2. 日志内容：时间、级别、文件、行号、内容
3. 日志级别：debug < info < warn < error < fatal
4. 日志翻滚：设置日志大小
