# 实时信号采集系统 - 项目结构

## 项目概述

基于Qt5开发的实时信号采集Demo系统，支持32通道高速数据采集、实时处理、可视化和持久化。

## 目录结构

```
signal_acquisition_system/
├── signal_acquisition.pro      # Qt项目配置文件
├── main.cpp                    # 程序主入口
├── signal_acquisition.h        # 主窗口头文件
├── signal_acquisition.cpp      # 主窗口实现
├── datasource.h                # 数据源模拟头文件
├── datasource.cpp              # 数据源模拟实现
├── dataprocessor.h             # 数据处理头文件
├── dataprocessor.cpp           # 数据处理实现
├── datawriter.h                # 数据持久化头文件
├── datawriter.cpp              # 数据持久化实现
├── data/                       # 数据存储目录（运行时生成）
    └── signal_*.csv           # 采集的信号数据文件
```

## 文件说明

### 核心文件

#### 1. signal_acquisition.pro
Qt项目配置文件，定义了：
- Qt模块：core, gui, charts, widgets
- C++标准：C++11
- 源文件和头文件列表

#### 2. main.cpp
程序入口点，负责：
- 创建QApplication实例
- 初始化主窗口
- 启动事件循环

#### 3. mainwindow.h/cpp
主窗口模块，实现：
- **UI界面**：控制面板、参数设置、图表显示
- **图表可视化**：使用Qt Charts实时显示波形
- **用户交互**：开始/停止、参数调整、通道切换
- **状态监控**：采集状态、告警提示

#### 4. datasource.h/cpp
数据源模拟模块，功能：
- **信号生成**：32通道模拟信号（正弦波+噪声）
- **采样控制**：51.2KHz采样率
- **峰值模拟**：随机产生突发峰值
- **定时器驱动**：使用QTimer定时生成数据

#### 5. dataprocessor.h/cpp
数据处理模块，功能：
- **滑动窗口滤波**：可配置窗口大小的移动平均滤波
- **峰值检测**：阈值检测和告警触发
- **线程安全**：使用QMutex保护共享数据
- **实时处理**：处理延迟 < 5ms

#### 6. datawriter.h/cpp
数据持久化模块，功能：
- **CSV格式写入**：原始数据异步写入文件
- **文件分段**：每5分钟自动创建新文件
- **写入控制**：支持暂停/恢复写入
- **线程安全**：使用QMutex保护文件操作

### 数据目录

#### data/
运行时自动创建的目录，存储采集的信号数据：
- 文件命名：`signal_yyyyMMdd_HHmmss.csv`
- 文件格式：CSV，包含时间戳和32通道数据
- 自动轮换：每5分钟生成新文件

## 模块依赖关系

```
main.cpp
    └── MainWindow
        ├── DataSource (数据源)
        │       └── emit dataReady → DataProcessor, DataWriter
        ├── DataProcessor (数据处理)
        │       └── emit processedReady → MainWindow
        │       └── emit alertTriggered → MainWindow
        └── DataWriter (数据持久化)
                └── 写入CSV文件

MainWindow (UI)
    ├── Qt Charts (可视化)
    └── 用户控件 (按钮、输入框等)
```

## 技术栈

- **框架**：Qt5
- **图表**：Qt Charts
- **C++标准**：C++11
- **并发**：QTimer, QMutex, 信号槽机制
- **数据格式**：CSV

## 核心功能对应文件

| 功能 | 实现文件 |
|------|---------|
| 实时数据采集 | datasource.cpp |
| 信号滤波处理 | dataprocessor.cpp |
| 峰值检测告警 | dataprocessor.cpp |
| 实时波形显示 | mainwindow.cpp |
| 数据持久化 | datawriter.cpp |
| 用户界面控制 | mainwindow.cpp |

## 编译和运行

### 环境要求
- Qt5.x
- Qt Charts模块
- C++11编译器

### 编译步骤
```bash
qmake signal_acquisition.pro
make
```

### 运行
```bash
./signal_acquisition
```

## 主要类说明

### MainWindow
主窗口类，继承自QMainWindow，管理整个应用的UI和逻辑。

### DataSource
数据源类，继承自QObject，使用QTimer定时生成模拟信号数据。

### DataProcessor
数据处理类，继承自QObject，实现滑动窗口滤波和峰值检测。

### DataWriter
数据写入类，继承自QObject，负责将采集数据写入CSV文件。

## 性能指标

- **采样率**：51.2 KHz
- **通道数**：32
- **处理延迟**：< 5ms
- **刷新率**：20 FPS
- **数据持久化**：异步写入，不阻塞主线程

## 扩展建议

1. 添加真实硬件数据源接口
2. 实现更多滤波算法（卡尔曼滤波、FFT等）
3. 支持多种数据格式导出
4. 添加数据回放功能
5. 实现网络数据传输
