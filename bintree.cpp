

#include <iostream>

#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <random>

using std::vector;
using std::string;
using std::cout;

template < typename T >
class BinTree {
    struct Node {
        T value;
        Node* left, * right;
        Node() : left(nullptr), right(nullptr) {}
        Node(const T& value) : value(value), left(nullptr), right(nullptr) {}
        ~Node() {
            delete left;
            delete right;
        }
        int max_depth() const {
            const int left_depth = left ? left->max_depth() : 0;
            const int right_depth = right ? right->max_depth() : 0;
            return (left_depth > right_depth ? left_depth : right_depth) + 1;
        }
    };

    Node* root;

public:
    BinTree() : root(nullptr) {}~BinTree() {
        delete root;
    }

    int get_max_depth() const {
        return root ? root->max_depth() : 0;
    }
    void clear() {
        delete root;
        root = nullptr;
    }
    void insert() {}
    template < typename...Args >
    void insert(const T& value, Args...more) {
        if (!root) {
            root = new Node(value);
        }
        else {
            Node* p = root;
            for (;;) {
                if (value == p->value) return;
                Node*& pchild = value < p->value ? p->left : p->right;
                if (!pchild) {
                    pchild = new Node(value);
                    break;
                }
                p = pchild;
            }
        }
        insert(more...);
    }

    struct cell_display {
        string valstr;
        bool present;
        cell_display() : present(false) {}
        cell_display(std::string valstr) : valstr(valstr), present(true) {}
    };

    using display_rows = vector < vector < cell_display > >;

    //  код генерации  является итеративным.

    // get_row_display строит вектор векторов структур cell_display
    // каждый вектор структур cell_display представляет одну строку, начиная с корня
    display_rows get_row_display() const {

        vector < Node* > traversal_stack;
        vector < std::vector < Node* > > rows;
        if (!root) return display_rows();

        Node* p = root;
        const int max_depth = root->max_depth();
        rows.resize(max_depth);
        int depth = 0;
        for (;;) {

            if (depth == max_depth - 1) {
                rows[depth].push_back(p);
                if (depth == 0) break;
                --depth;
                continue;
            }

            if (traversal_stack.size() == depth) {
                rows[depth].push_back(p);
                traversal_stack.push_back(p);
                if (p) p = p->left;
                ++depth;
                continue;
            }

            if (rows[depth + 1].size() % 2) {
                p = traversal_stack.back();
                if (p) p = p->right;
                ++depth;
                continue;
            }


            if (depth == 0) break;

            traversal_stack.pop_back();
            p = traversal_stack.back();
            --depth;
        }


        display_rows rows_disp;
        std::stringstream ss;
        for (const auto& row : rows) {
            rows_disp.emplace_back();
            for (Node* pn : row) {
                if (pn) {
                    ss << pn->value;
                    rows_disp.back().push_back(cell_display(ss.str()));
                    ss = std::stringstream();
                }
                else {
                    rows_disp.back().push_back(cell_display());
                }
            }
        }
        return rows_disp;
    }


    vector < string > row_formatter(const display_rows& rows_disp) const {
        using s_t = string::size_type;

        s_t cell_width = 0;
        for (const auto& row_disp : rows_disp) {
            for (const auto& cd : row_disp) {
                if (cd.present && cd.valstr.length() > cell_width) {
                    cell_width = cd.valstr.length();
                }
            }
        }

        // что ширина_ячейки - нечетное число
        if (cell_width % 2 == 0) ++cell_width;

        // позволяет соединять узлы листьев, если
        // все они имеют размер в один символ
        if (cell_width < 3) cell_width = 3;

        // formatted_rows будет содержать результаты
        vector < string > formatted_rows;


        s_t row_count = rows_disp.size();

        // количество элементов  строки кратное двум
        s_t row_elem_count = 1 << (row_count - 1);

        // left_pad содержит количество пробельных символов в начале нижнего ряда
        s_t left_pad = 0;


        for (s_t r = 0; r < row_count; ++r) {
            const auto& cd_row = rows_disp[row_count - r - 1];
            s_t space = (s_t(1) << r) * (cell_width + 1) / 2 - 1;
            string row;
            for (s_t c = 0; c < row_elem_count; ++c) {
                row += string(c ? left_pad * 2 + 1 : left_pad, ' ');
                if (cd_row[c].present) {
                    const string& valstr = cd_row[c].valstr;

                    s_t long_padding = cell_width - valstr.length();
                    s_t short_padding = long_padding / 2;
                    long_padding -= short_padding;
                    row += string(c % 2 ? short_padding : long_padding, ' ');
                    row += valstr;
                    row += string(c % 2 ? long_padding : short_padding, ' ');
                }
                else {
                    row += string(cell_width, ' ');
                }
            }
            formatted_rows.push_back(row);

            if (row_elem_count == 1) break;

            // Добавляем строки символов прямой и обратной косой черты на расстоянии друг от друга
             // для «соединения» строк значений узла двух строк.
             //  space - количество   строк.
            s_t left_space = space + 1;
            s_t right_space = space - 1;
            for (s_t sr = 0; sr < space; ++sr) {
                string row;
                for (s_t c = 0; c < row_elem_count; ++c) {
                    if (c % 2 == 0) {
                        row += string(c ? left_space * 2 + 1 : left_space, ' ');
                        row += cd_row[c].present ? '/' : ' ';
                        row += string(right_space + 1, ' ');
                    }
                    else {
                        row += string(right_space, ' ');
                        row += cd_row[c].present ? '\\' : ' ';
                    }
                }
                formatted_rows.push_back(row);
                ++left_space;
                --right_space;
            }
            left_pad += space + 1;
            row_elem_count /= 2;
        }

        // корневой узел в начало (сверху)
        std::reverse(formatted_rows.begin(), formatted_rows.end());

        return formatted_rows;
    }

    // Обрезает равное количество пробельных символов из
    // начала каждой строки в векторе.
    // По крайней мере одна строка в векторе будет начинаться
    // без пробельных символов.
    static void trim_rows_left(vector < string >& rows) {
        if (!rows.size()) return;
        auto min_space = rows.front().length();
        for (const auto& row : rows) {
            auto i = row.find_first_not_of(' ');
            if (i == string::npos) i = row.length();
            if (i == 0) return;
            if (i < min_space) min_space = i;
        }
        for (auto& row : rows) {
            row.erase(0, min_space);
        }
    }

    // Выдаем представление дерева в cout
    void Dump() const {
        const int d = get_max_depth();

        // Если дерево пустое, сообщите об этом кому-нибудь
        if (d == 0) {
            cout << " <пустое дерево>\n";
            return;
        }

        // Это дерево не пустое, поэтому получаем список значений узлов...
        const auto rows_disp = get_row_display();
        // Затем отформатируйте их в текстовое представление...
        auto formatted_rows = row_formatter(rows_disp);
        // затем обрезаем лишние пробельные символы с левой стороны текста...
        trim_rows_left(formatted_rows);
        // затем выводим текст в cout.
        for (const auto& row : formatted_rows) {
            std::cout << ' ' << row << '\n';
        }
    }
};

int main() {
    BinTree < int > bt;

    // Build OP's tree
    bt.insert(8, 5, 2, 6, 10, 9, 11);
    cout << "Tree from OP:\n\n";
    bt.Dump();
    cout << "\n\n";

    bt.clear();


    std::random_device rd;
    std::mt19937 rng(rd());

    int MaxCount = 20;
    int MaxDepth = 5;
    const int Min = 0,
        Max = 1000;

    std::uniform_int_distribution < int > dist(Min, Max);

    while (MaxCount--) {
        bt.insert(dist(rng));
        if (bt.get_max_depth() >= MaxDepth) break;
    }

    cout << "Randomly generated tree:\n\n";
    bt.Dump();
}
