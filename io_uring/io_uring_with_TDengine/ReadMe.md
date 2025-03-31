dpdk_taos_demo/
├── main.c                  # 主入口，初始化线程
├── dpdk_rx.c              # DPDK 初始化与收包逻辑
├── taos_writer.c          # io_uring + TDengine 插入逻辑
├── ringbuf.c / ringbuf.h  # 简单线程安全环形缓冲队列
├── common.h               # 公共结构体、字段定义
├── Makefile               # 构建
└── README.md              # 使用说明

功能说明
模块	功能
dpdk_rx.c	使用 DPDK 初始化网卡，循环收包，提取 src_ip、dst_ip、数据长度
ringbuf	简化环形缓冲区，用于线程通信
taos_writer.c	独立线程，读取 ring 中的数据，用 TDengine 插入
io_uring	主要用于管理异步线程任务，如文件写入调试日志等（可选）
TDengine C API	用于实际执行 taos_query() 或 taos_stmt 批量写入

