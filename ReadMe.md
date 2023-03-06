#项目简述
###包含功能和目的
  软渲染器，包括一些经典的光照模型，现代的pbr模型和npr模型，包含简单的渲染管线(前向渲染和延迟渲染)，一些抗锯齿以及后处理
以及一些图形API引入，并用统一框架整合，最终可选择不同api着色
#项目进度
**3.4** 完成环境的配置，并且引入对应库
**3.6** 新增一些框架，在完善中，新增框架图
##一些常用命令
### windows平台（64）
cmake -B winbuild -S . "-DCMAKE_TOOLCHAIN_FILE=C:/Users/32672/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build winbuild

###mac平台

nothing

