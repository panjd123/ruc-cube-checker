#include <array>
#include <ctime>
#include <functional>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <stack>
#include <string>
#include <tuple>
#include <utility>

namespace CubeSolver {
using namespace std;

const string faces[] = {
    "back",
    "left",
    "up",
    "right",
    "front",
    "down",
};

// 用 std::array 方便引用和复制
using Surface = array<array<char, 3>, 3>;
using Cube = array<Surface, 6>;
Cube sfs;  // surfaces

void print(const Cube& sfs) {
    for (int i = 0; i < 6; i++) {
        cout << faces[i] << ":\n";
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                cout << sfs[i][j][k] << " ";
            }
            cout << endl;
        }
        cout << endl;
        // cout << "------------------------\n";
    }
}

void print(const char* path, const Cube& sfs) {
    freopen(path, "w", stdout);
    print(sfs);
}

bool check(const Cube& sfs) {
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                if (sfs[i][j][k] != sfs[i][0][0]) {
                    return false;
                }
            }
        }
    }
    return true;
}

// 获取 三连格 的引用,方便轮换
tuple<char&, char&, char&> get_3(Surface& sf, int op) {
    // 8=row, 9=col, 1=anti 即变化的下标从大到小
    switch (op) {
        case 81:  // r1
            return make_tuple(ref(sf[0][0]), ref(sf[0][1]), ref(sf[0][2]));
        case 82:  // r2
            return make_tuple(ref(sf[1][0]), ref(sf[1][1]), ref(sf[1][2]));
        case 83:  // r3
            return make_tuple(ref(sf[2][0]), ref(sf[2][1]), ref(sf[2][2]));
        case 91:  // c1
            return make_tuple(ref(sf[0][0]), ref(sf[1][0]), ref(sf[2][0]));
        case 92:  // c2
            return make_tuple(ref(sf[0][1]), ref(sf[1][1]), ref(sf[2][1]));
        case 93:  // c3
            return make_tuple(ref(sf[0][2]), ref(sf[1][2]), ref(sf[2][2]));
        case 181:  //-r1
            return make_tuple(ref(sf[0][2]), ref(sf[0][1]), ref(sf[0][0]));
        case 182:  //-r2
            return make_tuple(ref(sf[1][2]), ref(sf[1][1]), ref(sf[1][0]));
        case 183:  //-r3
            return make_tuple(ref(sf[2][2]), ref(sf[2][1]), ref(sf[2][0]));
        case 191:  //-c1
            return make_tuple(ref(sf[2][0]), ref(sf[1][0]), ref(sf[0][0]));
        case 192:  //-c2
            return make_tuple(ref(sf[2][1]), ref(sf[1][1]), ref(sf[0][1]));
        case 193:  //-c3
            return make_tuple(ref(sf[2][2]), ref(sf[1][2]), ref(sf[0][2]));
        default:
            throw "invalid op";
    }
}
// 轮换
void swap_4(Cube& sfs, string order, array<int, 4> ids, bool anti) {
    if (anti) {
        swap(ids[0], ids[3]);
        swap(ids[1], ids[2]);
        swap(order[0], order[3]);
        swap(order[1], order[2]);
    }
    for (int i = 3; i > 0; i--) {
        auto t1 = get_3(sfs[order[i] - '0'], ids[i]);
        auto t2 = get_3(sfs[order[i - 1] - '0'], ids[i - 1]);
        swap(get<0>(t1), get<0>(t2));
        swap(get<1>(t1), get<1>(t2));
        swap(get<2>(t1), get<2>(t2));
    }
}

// 旋转
void rotate(Cube& sfs, int idx, bool anti) {
    auto tmp = sfs;
    if (anti) {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                sfs[idx][i][j] = tmp[idx][2 - j][i];
            }
        }
    } else {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                sfs[idx][i][j] = tmp[idx][j][2 - i];
            }
        }
    }
}

// 以下都是编码,原含义见 encoding.txt
// 简单说, 利用ppt中的展开图, 其恰好对应习惯中的 wasd(xc)
// 其中 wasdx 是十字, c 定义为最下面的
// wasdxc 按顺序编码为 012345

// ORDERS 表示面的轮换顺序
const string ORDERS[] = {
    "0245",
    "0245",
    "0245",
    "0341",
    "0341",
    "0341",
    "1235",
    "1235",
    "1235",
};
// IDS 的含义见 get_3(), 用来表示3连块的6种位置和2种方向
const array<int, 4> IDS[] = {
    {91, 91, 91, 91},
    {92, 92, 92, 92},
    {93, 93, 93, 93},
    {81, 93, 183, 191},
    {82, 92, 182, 192},
    {83, 91, 181, 193},
    {83, 83, 83, 181},
    {82, 82, 82, 182},
    {81, 81, 81, 183},
};
// ROTATE 中非负数表示逆时针, 负数反之, 6表示无旋转
// 这里 0 恰好用不到它的顺时针旋转, 所以这个编码没法表示也没关系
const int ROTATE[] = {
    -1,
    6,
    3,
    5,
    6,
    -2,
    -4,
    6,
    0,
};

Cube get_sfs(string color) {
    Cube sfs;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                sfs[i][j][k] = color[i];
            }
        }
    }
    return sfs;
}

void act(Cube& sfs, int i, int anti) {
    swap_4(sfs, ORDERS[i], IDS[i], anti);
    if (ROTATE[i] != 6) {
        rotate(sfs, abs(ROTATE[i]), anti ^ (ROTATE[i] < 0));
    }
}

void act(Cube& sfs, string action) {
    int i = action[0] - '0';
    int anti = action[1] == '-';
    swap_4(sfs, ORDERS[i], IDS[i], anti);
    if (ROTATE[i] != 6) {
        rotate(sfs, abs(ROTATE[i]), anti ^ (ROTATE[i] < 0));
    }
}

void acts(Cube& sfs, string s) {
    string action;
    istringstream str(s);
    while (str >> action) {
        int i = action[0] - '0';
        int anti = action[1] == '-';
        swap_4(sfs, ORDERS[i], IDS[i], anti);
        if (ROTATE[i] != 6) {
            rotate(sfs, abs(ROTATE[i]), anti ^ (ROTATE[i] < 0));
        }
    }
}
const int N = 20000000;

vector<string> cube_patterns = {
    "ygpbwr",
    "yrgpwb",
    "grwpby",
    "wrbpyg",
    "brypgw",
    "rybwpg",
    "bypwgr",
    "pygwrb",
    "gyrwbp",
    "rybwpg",
    "bypwgr",
    "pygwrb",
};
vector<Cube> cubes;

/*
Cube sfs;
int direction;
State* from;
int action;
int anti;
int depth;
*/
struct State {
    Cube sfs;
    int direction;
    State* from;
    int action;
    int anti;
    int depth;
};

using hash_type = array<unsigned long long, 3>;

map<Cube, State*> visited;
map<hash_type, State*> visited2;
vector<State*> states;
queue<State*> Q_state;
string get_action_name(State* s, int op) {
    return to_string(s->action) + string(s->anti ^ op ? "-" : "+");
}
string link_path(Cube c1, Cube c2);
string get_path_link(State* s) {
    string ret = "";
    if (s->direction == 1) {
        stack<State*> ans;
        State* lsts = nullptr;
        while (s->depth > 0) {
            ans.push(s);
            s = s->from;
        }
        while (!ans.empty()) {
            if (lsts)
                ret += link_path(lsts->sfs, ans.top()->sfs) + " ";
            lsts = ans.top();
            ans.pop();
        }
        return ret;
    } else {
        queue<State*> ans;
        State* lsts = nullptr;
        while (s->depth > 0) {
            ans.push(s);
            s = s->from;
        }
        while (!ans.empty()) {
            if (lsts)
                ret += link_path(lsts->sfs, ans.front()->sfs) + " ";
            lsts = ans.front();
            ans.pop();
        }
        return ret;
    }
}
string get_path(State* s) {
    string ret = "";
    if (s->direction == 1) {
        stack<string> ans;
        while (s->depth > 0) {
            ans.push(get_action_name(s, 0));
            s = s->from;
        }
        while (!ans.empty()) {
            ret += ans.top() + " ";
            ans.pop();
        }
        return ret;
    } else {
        queue<string> ans;
        while (s->depth > 0) {
            ans.push(get_action_name(s, 1));
            s = s->from;
        }
        while (!ans.empty()) {
            ret += ans.front() + " ";
            ans.pop();
        }
        return ret;
    }
}
string bfs(vector<Cube> fr, vector<Cube> to);
string BFS() {
    clock_t start = clock();
    queue<State*> empty;
    swap(Q_state, empty);
    visited.clear();
    visited2.clear();
    states.clear();

    string ret = bfs(vector<Cube>({sfs}), cubes);
    // string ret = bfs(vector<Cube>({sfs}), vector<Cube>({cubes[0]}));

    clock_t end = clock();

    cout << "ans:" << ret << endl
         << "time: " << (double)(end - start) / CLOCKS_PER_SEC << endl
         << "states: " << states.size() << endl;

    for (auto s : CubeSolver::states) {
        delete s;
    }
    return ret;
}

void init() {
    int init_size = cube_patterns.size();
    for (int i = 0; i < init_size; i++) {
        auto sfs_pattern = cube_patterns[i];
        auto rev_sfs_pattern = sfs_pattern;
        swap(rev_sfs_pattern[0], rev_sfs_pattern[4]);
        swap(rev_sfs_pattern[1], rev_sfs_pattern[3]);
        cube_patterns.push_back(rev_sfs_pattern);
        cubes.push_back(
            get_sfs(sfs_pattern));
        cubes.push_back(
            get_sfs(rev_sfs_pattern));
    }
}

unsigned long long get_hash(Surface sf) {
    auto tr = [](char x) {
        if(x=='g') return 0; 
        if(x=='r') return 1;
        if(x=='b') return 2;
        if(x=='y') return 3;
        if(x=='w') return 4;
        if(x=='p') return 5;
        throw "invalid sf"; };
    unsigned long long ret = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            ret = tr(sf[i][j]) + ret * 6;
        }
    }
    return ret;
}

hash_type get_hash(Cube sfs) {
    map<char, unsigned long long> mp;
    for (int i = 0; i < 6; i++) {
        mp[cube_patterns[0][i]] = get_hash(sfs[i]);
    }
    array<unsigned long long, 6> mn;
    hash_type ret;
    mn.fill(0xffffffffffffffff);
    // for (auto s : cube_patterns)
    {
        auto s = cube_patterns[0];
        array<unsigned long long, 6> hash0;
        for (int i = 0; i < 6; i++) {
            hash0[i] = mp[s[i]];
        }
        mn = min(mn, hash0);
    }
    for (int i = 0; i < 3; i++) {
        ret[i] = (mn[2 * i] << 32) + mn[2 * i + 1];
    }
    return ret;
}
string link_path_ans = "";
void dfs(State* s, Cube c2);
string link_path(Cube c1, Cube c2) {
    link_path_ans = "";
    auto s = new State{c1, 1, nullptr, 0, 0, 0};
    dfs(s, c2);
    for (auto state : states) {
        delete state;
    }
    return link_path_ans;
}
void dfs(State* s, Cube c2) {
    if (s->depth > 6 || link_path_ans.length() > 0) {
        return;
    }
    // std::cout << "depth: " << << std::endl;

    for (int action = 0; action < 9; action++) {
        for (int anti = 0; anti < 2; anti++) {
            auto sfs_nxt = s->sfs;
            act(sfs_nxt, action, anti);
            auto s_nxt = new State{sfs_nxt, s->direction, s, action, anti, s->depth + 1};
            states.push_back(s_nxt);
            if (sfs_nxt == c2) {
                link_path_ans = get_path(s_nxt);
            } else {
                dfs(s_nxt, c2);
            }
        }
    }
}
string bfs(vector<Cube> fr, vector<Cube> to) {
    for (auto sfs : fr) {
        auto s = new State{sfs, 1, nullptr, 0, 0, 0};
        Q_state.push(s);
        visited2[get_hash(s->sfs)] = s;
        states.push_back(s);
    }
    for (auto sfs : to) {
        auto s0 = new State{sfs, -1, nullptr, 0, 0, 0};
        Q_state.push(s0);
        visited2[get_hash(s0->sfs)] = s0;
        states.push_back(s0);
    }
    while (!Q_state.empty()) {
        State* s = Q_state.front();
        Q_state.pop();
        for (int action = 0; action < 9; action++) {
            for (int anti = 0; anti < 2; anti++) {
                auto sfs_nxt = s->sfs;
                act(sfs_nxt, action, anti);
                auto hash = get_hash(sfs_nxt);

                if (visited2[hash] == nullptr || visited2[hash]->direction != s->direction) {
                    auto s_nxt = new State{sfs_nxt, s->direction, s, action, anti, s->depth + 1};
                    if (visited2[hash] == nullptr) {
                        visited2[hash] = s_nxt;
                        states.push_back(s_nxt);
                        Q_state.push(s_nxt);
                        if (Q_state.size() % 10000 == 0)
                            cout << Q_state.size() << " " << s_nxt->depth << endl;
                    } else {  // if (visited2[hash]->direction != s->direction) {
                        if (visited2[hash]->direction == 1) {
                            return get_path(visited2[hash]) + get_path(s_nxt);
                        } else {
                            return get_path(s_nxt) + get_path(visited2[hash]);
                        }
                    }
                } else {
                    continue;
                }
            }
        }
    }
    return "no ans";
}

void input(const char* path) {
    freopen(path, "r", stdin);
    for (int i = 0; i < 6; i++) {
        string s;
        cin >> s;
        int idx;
        if (s == "back:") {
            idx = 0;
        } else if (s == "down:") {
            idx = 5;
        } else if (s == "front:") {
            idx = 4;
        } else if (s == "left:") {
            idx = 1;
        } else if (s == "right:") {
            idx = 3;
        } else if (s == "up:") {
            idx = 2;
        } else {
            cout << "invalid action: " << s << endl;
            throw "invalid action";
        }
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                cin >> sfs[idx][j][k];
            }
        }
    }
}

void input_actions(const char* path) {
    freopen(path, "r", stdin);
    string s;
    while (cin >> s) {
        acts(sfs, s);
    }
}

}  // namespace CubeSolver
using namespace CubeSolver;
int main(int argc, char const* argv[]) {
    if (argc != 4) {
        cout << "Usage: " << argv[0] << " <input> <actions> <output>" << endl;
        return 1;
    }
    init();
    CubeSolver::input(argv[1]);
    CubeSolver::input_actions(argv[2]);
    CubeSolver::print(argv[3], CubeSolver::sfs);
}