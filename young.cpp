#include <iostream>
#include <vector>
#include <boost/multiprecision/cpp_int.hpp>

using boost::multiprecision::cpp_int;
using namespace std;

// НСД для великих цiлих чисел
cpp_int bigint_gcd(cpp_int a, cpp_int b) {
    while (b != 0) {
        cpp_int t = a % b;
        a = b;
        b = t;
    }
    return a;
}

int main() {
    int N;   // параметр групи SU(N)
    int r;   // кiлькiсть рядкiв дiаграми

    // === IНСТРУКЦIЯ ДЛЯ КОРИСТУВАЧА ===
    cout << "==============================================" << endl;
    cout << "   КАЛЬКУЛЯТОР ДIАГРАМ ЮНГА ДЛЯ SU(N)" << endl;
    cout << "==============================================" << endl;
    cout << "Програма обчислює розмiрнiсть незводимого" << endl;
    cout << "представлення групи SU(N) за заданою" << endl;
    cout << "дiаграмою Юнга (довжинами її рядкiв)." << endl << endl;

    cout << "Формат введення даних:" << endl;
    cout << "  1) Цiле число N >= 2  — параметр групи SU(N)." << endl;
    cout << "  2) Цiле число r       — кiлькiсть рядкiв дiаграми." << endl;
    cout << "  3) r цiлих чисел      — довжини кожного рядка" << endl;
    cout << "     (через пробiл, зверху донизу)." << endl;
    cout << "     Довжина нижнього рядка не повинна" << endl;
    cout << "     перевищувати довжину рядка вище." << endl << endl;

    cout << "Приклади:" << endl;
    cout << "  * Одна клiтина (фундаментальне):" << endl;
    cout << "      N = 3, r = 1, рядки: 1" << endl;
    cout << "  * Антисиметричний тензор 2-го рангу (1,1):" << endl;
    cout << "      N = 3, r = 2, рядки: 1 1" << endl << endl;

    cout << "Тепер введiть данi у вказаному порядку." << endl << endl;

    cout << "Введiть N (для SU(N)): " << flush;
    if (!(cin >> N)) return 0;

    cout << "Введiть кiлькiсть рядкiв дiаграми r: " << flush;
    cin >> r;

    vector<int> row(r);
    cout << "Введiть довжини рядкiв (через пробiл, зверху донизу): " << flush;
    for (int i = 0; i < r; ++i) {
        cin >> row[i];
    }

    // Пiдрахунок загальної кiлькостi клiтин
    int totalBoxes = 0;
    for (int len : row) totalBoxes += len;

    vector<cpp_int> num; // чисельники: N + j - i
    vector<cpp_int> den; // знаменники: довжини гачкiв
    num.reserve(totalBoxes);
    den.reserve(totalBoxes);

    // Обходимо всi клiтини дiаграми Юнга
    for (int i = 0; i < r; ++i) {          // i — iндекс рядка (0 — верхнiй)
        for (int j = 0; j < row[i]; ++j) { // j — iндекс стовпця (0 — лiвий)
            int right = row[i] - j - 1;    // клiтини праворуч
            int below = 0;                 // клiтини нижче
            for (int k = i + 1; k < r; ++k) {
                if (row[k] > j) ++below;
            }

            int hook = 1 + right + below;      // довжина «гачка»
            cpp_int n = cpp_int(N + j - i);    // N + (j - i)

            num.push_back(n);
            den.push_back(cpp_int(hook));
        }
    }

    // Скорочуємо дрiб: дiлимо кожен знаменник на НСД з якимось чисельником
    int m = (int)den.size();
    int k = (int)num.size();

    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < k; ++j) {
            if (den[i] == 1) break;
            cpp_int g = bigint_gcd(den[i], num[j]);
            if (g > 1) {
                den[i] /= g;
                num[j] /= g;
            }
        }
    }

    // Перемножуємо всi чисельники — отримуємо цiлу розмiрнiсть
    cpp_int dim = 1;
    for (const auto &x : num) {
        dim *= x;
    }

    cout << endl << "Розмiрнiсть представлення: " << dim << endl;

    return 0;
}
