#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <boost/multiprecision/cpp_int.hpp>

using namespace std;
using boost::multiprecision::cpp_int;

// ---------- НСД для великих цілих ----------
cpp_int bigint_gcd(cpp_int a, cpp_int b) {
    while (b != 0) {
        cpp_int t = a % b;
        a = b;
        b = t;
    }
    return a;
}

// ---------- Розмірність представлення по діаграмі Юнга ----------
cpp_int compute_dimension(int N, const vector<int>& row) {
    int r = (int)row.size();
    int totalBoxes = 0;
    for (int x : row) totalBoxes += x;

    vector<cpp_int> num; // чисельники N + j - i
    vector<cpp_int> den; // знаменники (довжини гачків)
    num.reserve(totalBoxes);
    den.reserve(totalBoxes);

    for (int i = 0; i < r; ++i) {          // індекс рядка (0 зверху)
        for (int j = 0; j < row[i]; ++j) { // індекс стовпця (0 зліва)
            int right = row[i] - j - 1;
            int below = 0;
            for (int k = i + 1; k < r; ++k) {
                if (row[k] > j) ++below;
            }
            int hook = 1 + right + below;

            cpp_int n = cpp_int(N + j - i); // N + (j - i)
            num.push_back(n);
            den.push_back(cpp_int(hook));
        }
    }

    // скорочуємо дроби
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

    cpp_int dim = 1;
    for (const auto& x : num) dim *= x;
    return dim;
}

// ---------- Структура таблиці Юнга з мітками (для продукту) ----------
struct Tableau {
    vector<int> rowLen;             // довжини рядків
    vector<vector<int>> labels;     // тієї ж форми; 0 — клітини з A, >0 — з B
};

// ключ для map по формі діаграми
string shape_key(const vector<int>& row) {
    string s;
    for (size_t i = 0; i < row.size(); ++i) {
        if (i) s.push_back(',');
        s += to_string(row[i]);
    }
    return s;
}

// ---------- Перевірка, чи можна додати клітину з даною міткою ----------
bool can_add_box(const Tableau& T, int N, int rowIndex, int label) {
    int numRows = (int)T.rowLen.size();
    bool newRow = (rowIndex == numRows);

    int col = newRow ? 1 : (T.rowLen[rowIndex] + 1); // індекс стовпця (1..)

    // форма діаграми: довжина рядка не може перевищувати довжину рядка вище
    if (!newRow && rowIndex > 0 && T.rowLen[rowIndex - 1] < col)
        return false;

    // перевірка стовпця: заборонено повторну мітку в одному стовпці
    int heightBefore = 0;
    for (int i = 0; i < numRows; ++i) {
        if (T.rowLen[i] >= col) {
            ++heightBefore;
            if (T.labels[i][col - 1] == label) return false;
        }
    }

    // обмеження SU(N): висота стовпця <= N
    int heightAfter = heightBefore + 1;
    if (heightAfter > N) return false;

    return true;
}

// ---------- Рекурсивне додавання k клітин з однією міткою ----------
void add_boxes_label(const Tableau& T, int N, int label, int remaining,
                     vector<Tableau>& out) {
    if (remaining == 0) {
        out.push_back(T);
        return;
    }

    int numRows = (int)T.rowLen.size();

    // можемо додавати в будь-який існуючий рядок або створити новий внизу
    for (int r = 0; r <= numRows; ++r) {
        if (!can_add_box(T, N, r, label)) continue;

        Tableau T2 = T;
        if (r == numRows) {
            // новий рядок
            T2.rowLen.push_back(1);
            T2.labels.push_back(vector<int>(1, label));
        } else {
            T2.rowLen[r]++;
            T2.labels[r].push_back(label);
        }
        add_boxes_label(T2, N, label, remaining - 1, out);
    }
}

// ---------- Умова Яманоучі (балотна) для послідовності міток ----------
bool is_yamanouchi(const Tableau& T, int numLabels) {
    vector<int> seq;
    // читаємо мітки зліва-направо: рядки зверху вниз, у кожному справа наліво
    int rows = (int)T.rowLen.size();
    for (int i = 0; i < rows; ++i) {
        for (int j = T.rowLen[i] - 1; j >= 0; --j) {
            int lab = T.labels[i][j];
            if (lab > 0) seq.push_back(lab); // тільки клітини з B
        }
    }

    vector<int> cnt(numLabels + 2, 0);

    for (int x : seq) {
        cnt[x]++;
        // на кожному префіксі: #a_i >= #a_{i+1}
        for (int k = 1; k < numLabels; ++k) {
            if (cnt[k] < cnt[k + 1]) return false;
        }
    }
    return true;
}

// ---------- Головна програма ----------
int main() {
    cout << "==============================================\n";
    cout << "   КАЛЬКУЛЯТОР ТЕНЗОРНОГО ДОБУТКУ ДIАГРАМ ЮНГА\n";
    cout << "       ДЛЯ ПРЕДСТАВЛЕНЬ SU(N)\n";
    cout << "==============================================\n\n";

    cout << "Введення:\n";
    cout << "  N  — параметр групи SU(N)\n";
    cout << "  Дiаграма A: кiлькiсть рядкiв та довжини рядкiв\n";
    cout << "  Дiаграма B: кiлькiсть рядкiв та довжини рядкiв\n\n";

    int N;
    cout << "Введiть N (для SU(N)): ";
    if (!(cin >> N)) return 0;

    int rA;
    cout << "Введiть кiлькiсть рядкiв дiаграми A: ";
    cin >> rA;
    vector<int> A(rA);
    cout << "Введiть довжини рядкiв A (через пробiл, зверху донизу): ";
    for (int i = 0; i < rA; ++i) cin >> A[i];

    int rB;
    cout << "Введiть кiлькiсть рядкiв дiаграми B: ";
    cin >> rB;
    vector<int> B(rB);
    cout << "Введiть довжини рядкiв B (через пробiл, зверху донизу): ";
    for (int i = 0; i < rB; ++i) cin >> B[i];

    // Початкова таблиця: тільки діаграма A, усі мітки 0
    Tableau base;
    base.rowLen = A;
    base.labels.resize(rA);
    for (int i = 0; i < rA; ++i) {
        base.labels[i].assign(A[i], 0);
    }

    // Послідовно додаємо ряди B: a1, a2, ...
    vector<Tableau> current;
    current.push_back(base);

    for (int lab = 1; lab <= rB; ++lab) {
        vector<Tableau> next;
        int count = B[lab - 1]; // скільки клітин з міткою lab
        for (const auto& T : current) {
            vector<Tableau> tmp;
            add_boxes_label(T, N, lab, count, tmp);
            next.insert(next.end(), tmp.begin(), tmp.end());
        }
        current.swap(next);
    }

    // Фільтруємо за умовою Яманоучі
    vector<Tableau> valid;
    for (const auto& T : current) {
        if (is_yamanouchi(T, rB)) {
            valid.push_back(T);
        }
    }

    // Групуємо за формою діаграми: мультиплічності
    struct Info {
        int mult = 0;
        vector<int> shape;
    };

    map<string, Info> result;

    for (const auto& T : valid) {
        string key = shape_key(T.rowLen);
        auto& info = result[key];
        if (info.shape.empty()) info.shape = T.rowLen;
        info.mult += 1;
    }

    // Розмірності A та B
    cpp_int dimA = compute_dimension(N, A);
    cpp_int dimB = compute_dimension(N, B);

    cout << "\n----------------------------------------------\n";
    cout << "Результат розкладу тензорного добутку A ⊗ B:\n\n";

    cpp_int sumDims = 0;

    for (const auto& kv : result) {
        const Info& info = kv.second;
        cpp_int dimRep = compute_dimension(N, info.shape);

        cout << "Форма λ = (";
        for (size_t i = 0; i < info.shape.size(); ++i) {
            if (i) cout << ", ";
            cout << info.shape[i];
        }
        cout << "):  кратнiсть m = " << info.mult
             << ",  dim(λ) = " << dimRep << "\n";

        sumDims += dimRep * info.mult;
    }

    cout << "\n----------------------------------------------\n";
    cout << "Перевiрка розмiрностей:\n";
    cout << "  dim(A) = " << dimA << "\n";
    cout << "  dim(B) = " << dimB << "\n";
    cout << "  dim(A) * dim(B) = " << (dimA * dimB) << "\n";
    cout << "  Σ m_λ * dim(λ)   = " << sumDims << "\n";

    if (dimA * dimB == sumDims)
        cout << "  ✓ Спiвпадає (розклад коректний)\n";
    else
        cout << "  ⚠ НЕ спiвпадає (можлива помилка або занадто великий приклад)\n";

    return 0;
}
