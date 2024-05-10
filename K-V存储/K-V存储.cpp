#include "SkipList.h"
#include "DataBase.h"
#include <chrono>
#include <functional>
#include "Parallel_SkipList.h"
#include "Parallel_DataBase.h"
#include <thread>

const int numThreads = 100;
void transaction(Parallel_DataBase<int, std::string, Parallel_SkipList<int, std::string>>& db) {
    // 生成指定长度的随机字符串
    std::function<std::string(void)> generateRandomString = []()->std::string {
        int length = rand() % 10;
        const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
        std::string result;
        for (int i = 0; i < length; ++i) {
            result += charset[rand() % charset.length()];
        }
        return result;
    };
    std::function<int(void)> generateRandomInt = []()->int {
        return rand() * 1000 + rand();
    };
    //int x = rand() % 4;
    int x = 1;
    switch (x) {
        case 0:db.Insert(generateRandomInt(), generateRandomString()); break;
        case 1:db.Delete(generateRandomInt()); break;
        case 2:db.Search(generateRandomInt()); break;
        case 3:db.Revise(generateRandomInt(), generateRandomString()); break;
        case 4:db.RangeIn(generateRandomInt(), generateRandomInt()); break;
    }
}
void test(Parallel_DataBase<int, std::string, Parallel_SkipList<int, std::string>>& db)
{
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1'00; i++) {
        transaction(db);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "程序运行时间为: " << elapsed.count() << " 秒" << std::endl;
}
void Parallel_Test() {
    Parallel_DataBase<int, std::string, Parallel_SkipList<int, std::string>> db("storage");
    db.Start();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.push_back(std::thread(test,ref(db)));
    }
    for (std::thread& t : threads) {
        t.join();
    }
    for (auto& x : db.res) {
        std::cout << x << " ";
    }
    db.Close();
}
//int main() {
//    Parallel_Test();
//}