#include <rime_api_deprecated.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>


int main(int argc, char* argv[]) {
    // ===================== 1. 初始化参数（兼容不同版本） =====================
    RimeTraits traits;
    // 清空结构体（避免随机值导致初始化失败）
    memset(&traits, 0, sizeof(RimeTraits));
    // 词库目录（pacman 安装的默认路径，务必确认存在）
    traits.shared_data_dir = "/usr/share/rime-data";
    // 用户数据目录（自动创建，无需提前建）
    traits.user_data_dir = "./rime_user_data";
    traits.distribution_name = "rime_demo";
    traits.distribution_version = "1.0";

    // ===================== 2. 初始化引擎（核心函数） =====================
    std::cout << "正在初始化 RIME 引擎...\n";
    RimeInitialize(&traits);
    // if (!RimeInitialize(&traits)) {
    //     std::cerr << "❌ RIME 初始化失败！检查词库路径：" << traits.shared_data_dir << std::endl;
    //     return -1;
    // }
    std::cout << "✅ RIME 引擎初始化成功\n";

    // ===================== 3. 创建会话 =====================
    RimeSessionId session = RimeCreateSession();
    if (!session) {
        std::cerr << "❌ 创建 RIME 会话失败！" << std::endl;
        RimeFinalize();
        return -1;
    }

    // ===================== 4. 切换拼音方案（先安装 rime-data-luna-pinyin） =====================
    // 先检查拼音方案文件是否存在
    if (access("C:/software/msys64/mingw64/share/rime-data/luna_pinyin.schema.yaml", F_OK) != 0) {
        std::cerr << "❌ 未找到拼音方案！请执行：sudo pacman -S rime-data-luna-pinyin" << std::endl;
        RimeDestroySession(session);
        RimeFinalize();
        return -1;
    }

    //RimeSchema schema;
    //memset(&schema, 0, sizeof(RimeSchema));
    //schema.schema_id = "luna_pinyin";  // 朙月拼音方案
    if (!RimeSelectSchema(session, "luna_pinyin")) {
        std::cerr << "❌ 切换拼音方案失败！" << std::endl;
        RimeDestroySession(session);
        RimeFinalize();
        return -1;
    }
    std::cout << "✅ 已切换到【朙月拼音】方案\n";

    // ===================== 5. 模拟输入 "nihao" =====================
    std::string input = "nihao";
    std::cout << "\n输入编码：" << input << "\n";
    for (char c : input) {
        // 模拟按键输入（第二个参数是字符，第三个是修饰符（0=无））
        RimeProcessKey(session, c, 0);
    }
    usleep(200000);

    // ===================== 6. 获取候选词 =====================
    std::vector<std::string> candidates;
    RimeCandidateListIterator iterator;
    if (!RimeCandidateListBegin(session, &iterator)) {
        std::cerr << "遍历候选词失败！" << std::endl;
        RimeDestroySession(session);
        RimeFinalize();
        return -1;
    }
    do
    {
        //if (iterator->candidate && iterator->candidate.text) {  // 防空指针
            candidates.push_back(std::string(iterator.candidate.text));
        //}
    } while (RimeCandidateListNext(&iterator));

    // 输出候选词
    std::cout << "候选词列表：\n";
    if (candidates.empty()) {
        std::cout << "  （无候选词）\n";
    } else {
        for (size_t i = 0; i < candidates.size(); ++i) {
            std::cout << "  " << i+1 << ". " << candidates[i] << "\n";
        }
    }

    // ===================== 7. 清理资源 =====================
    RimeDestroySession(session);
    RimeFinalize();
    std::cout << "\n✅ RIME 引擎已关闭\n";

    return 0;
}