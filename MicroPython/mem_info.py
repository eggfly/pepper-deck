import os
import gc

def check_storage(path='/'):
    fs_stats = os.statvfs(path)
    
    # 计算存储空间参数
    block_size = fs_stats[0]         # 文件系统块大小（字节）
    total_blocks = fs_stats[2]       # 总数据块数
    free_blocks = fs_stats[3]        # 可用块数

    total_space = block_size * total_blocks
    free_space = block_size * free_blocks
    used_space = total_space - free_space
    
    print(f"存储路径: {path}")
    print(f"总空间: {total_space/1024:.2f} KB")
    print(f"已用空间: {used_space/1024:.2f} KB") 
    print(f"剩余空间: {free_space/1024:.2f} KB")
    print(f"使用率: {used_space/total_space:.1%}")
    print(gc.mem_free())
# 使用示例
check_storage()
