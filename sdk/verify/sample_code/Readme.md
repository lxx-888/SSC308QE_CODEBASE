sample_code下分为参考代码、lib、业务app，三部分，子项目以文件夹区分，如下所示：

    verify

        |-----prebuild_libs //第三方开源库，或者是sigmastar开发的闭源库，库的存放位置按照toolchain划分，例如glibc-9.1.0的版本： lib/glibc/9.1.0

        |-----common        // 项目开发中使用到公共代码，与mi无关，与chip无关，例如rtsp

        |-----sample_code

            |-------->source/$(CHIP)/   // 主要是各个模块demo代码参考

            |-------->source/internal/  // 参考代码的公共代码

            |--------libraries/          // 中间层框架代码，若未开源则只有头文件

            |--------applications/       // 基于中间件的各个业务层代码

            |--------build/             // 存放Makefile所需要的所有配置文件

            |--------Makefile           // 全局的makefile，可以管理所有的子项目

            |--------out

                |--------app/    // 编译出来的临时文件夹，存放编译输出的结果。

                |--------lib/    // 存放编译common、internal、中间层生成的lib
