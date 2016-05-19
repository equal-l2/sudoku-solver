#include <iostream>
#include <iomanip>
#include <array>
#include <bitset>

struct sudoku_cell{
    using numset = std::bitset<9>;
    sudoku_cell():cells{}{}
    std::array<unsigned,81> cells;
    std::array<numset,81> cands;
    std::array<numset,9> reduced_row;
    std::array<numset,9> reduced_col;

    inline unsigned rc_to_idx(const unsigned row, const unsigned col){
        return 9*row+col;
    }

    inline unsigned idx_to_row(const unsigned idx){
        return idx/9;
    }

    inline unsigned idx_to_col(const unsigned idx){
        return idx%9;
    }

    numset cand_in_row(const unsigned idx){
        numset ret;
        ret.set();
        const unsigned row = idx/9;
        for(unsigned i=row*9;i<row*9+9;++i) if(cells[i] != 0) ret.reset(cells[i]-1);
        return ret;
    }

    numset cand_in_col(const unsigned idx){
        numset ret;
        ret.set();
        const unsigned col = idx%9;
        for(unsigned i=col;i<81;i+=9) if(cells[i] != 0) ret.reset(cells[i]-1);
        return ret;
    }

    numset cand_in_3x3(const unsigned idx){
        numset ret;
        ret.set();
        const unsigned row = idx/9;
        const unsigned col = idx%9;
        const unsigned row_begin = row/3*3;
        const unsigned col_begin = col/3*3;
        for(unsigned i=row_begin;i<row_begin+3;++i){
            for(unsigned j=col_begin;j<col_begin+3;++j){
                if(cells[9*i+j] != 0) ret.reset(cells[9*i+j]-1);
            }
        }

        return ret;
    }

    inline unsigned num_of_onbit(const numset& e){ // return least canding number indicated in e
        for(unsigned i=0;i<9;++i) if(e[i]) return i+1;
        return 0; // unreachable
    }

    void solve(){
        gen_cands();
        BEGIN:
            if(reduce_line())           goto BEGIN;
            if(solve_3x3())             goto BEGIN;
            if(solve_one_candidate())   goto BEGIN;
    }

    bool solve_one_candidate(){ // solve cells which has only one candidate
        bool changed = false;
        for(unsigned i=0;i<81;++i){
            if(cells[i] == 0){
                if(cands[i].count() == 1){
                    cells[i] = num_of_onbit(cands[i]);
                    rebuild_cand(i);
                    changed = true;
                }
            }
        }
        return changed;
    }

    bool solve_3x3(){
        bool changed = false;
        for(unsigned i=0;i<3;++i){
            for(unsigned j=0;j<3;++j){
                if(solve_each_3x3(i,j)) changed = true;
            }
        }
        return changed;
    }

    bool solve_each_3x3(const unsigned row, const unsigned col){
        bool changed = false;
        for(unsigned i=0;i<9;++i){
            bool found = false;
            unsigned idx = 0;
            for(unsigned j=3*row;j<3*row+3;++j){
                for(unsigned k=3*col;k<3*col+3;++k){
                    auto idx_ = 9*j+k;
                    if(cands[idx_][i]){
                        if(found) goto LEND;
                        found = true;
                        idx = idx_;
                    }
                }
            }
            if(cells[idx] == 0 && found){
                cells[idx] = i+1;
                rebuild_cand(idx);
                changed = true;
            }
            LEND: ;
        }
        return changed;
    }

    bool reduce_line(){
        bool changed = false;
        for(unsigned i=0;i<3;++i){
            for(unsigned j=0;j<3;++j){
                if(reduce_line_3x3(i,j)) changed = true;
            }
        }
        return changed;
    }

    bool reduce_line_3x3(const unsigned row, const unsigned col){
        bool changed = false;
        for(unsigned i=0;i<9;++i){
            for(unsigned j=3*row;j<3*row+3;++j){
                auto idx = 9*j+3*col;
                if(
                    !reduced_row[j][i] &&
                    (
                        (cands[idx][i] && (cands[idx+1][i] || cands[idx+2][i])) || (cands[idx+1][i] && cands[idx+2][i])
                    )
                ){
                    for(unsigned k=3*row;k<3*row+3;++k){
                        if(j != k){
                            for(unsigned l=3*col;l<3*col+3;++l){
                                if(cands[9*k+l][i]) goto LEND;
                            }
                        }
                    }
                    for(unsigned k=9*j;k<9*j+9;++k){
                        if(k < 9*j+3*col || k >= 9*j+3*col+3) cands[k].reset(i);
                    }
                    reduced_row[j].set(i);
                    changed = true;
                    goto LEND;
                }
            }

            for(unsigned j=3*col;j<3*col+3;++j){
                auto idx = 27*row+j;
                if(
                    !reduced_col[j][i] &&
                    (
                        (cands[idx][i] && (cands[idx+9][i] || cands[idx+18][i])) || (cands[idx+9][i] && cands[idx+18][i])
                    )
                ){
                    for(unsigned k=3*row;k<3*row+3;++k){
                        for(unsigned l=col*3;l<col*3+3;++l){
                            if(j == l) continue;
                            if(cands[9*k+l][i]) goto LEND;
                        }
                    }
                    for(unsigned k=j%9;k<81;k+=9){
                        if(k < j || k > j+18) cands[k].reset(i);
                    }
                    reduced_col[j].set(i);
                    changed = true;
                    goto LEND;
                }
            }

            LEND: ;
        }
        return changed;
    }

    numset gen_cand(unsigned idx){
        return (cells[idx] != 0 ? numset().reset() : (cand_in_row(idx) & cand_in_col(idx) & cand_in_3x3(idx)));
    }

    void gen_cands(){
        for(unsigned i=0; i<81;++i) cands[i] = gen_cand(i);
    }

    void rebuild_cand(unsigned idx){
        const unsigned val = cells[idx];
        const unsigned row = idx/9;
        const unsigned col = idx%9;
        const unsigned row_begin = row/3*3;
        const unsigned col_begin = col/3*3;
        cands[idx].reset();
        for(unsigned i=row*9;i<row*9+9;++i) cands[i].reset(val-1);
        for(unsigned i=col;i<81;i+=9)    cands[i].reset(val-1);
        for(unsigned i=row_begin;i<row_begin+3;++i){
            for(unsigned j=col_begin;j<col_begin+3;++j){
                cands[9*i+j].reset(val-1);
            }
        }
    }

    friend std::ostream& operator<<(std::ostream& os, sudoku_cell& s){
        for (unsigned i=0;i<9;++i){
            os << ' ';
            for(unsigned j=0;j<9;++j){
                os << s.cells[9*i+j] << ((j+1)%3 == 0 && j != 8 ? " | " : " ");
            }
            std::cout << (((i+1)%3 == 0 && i != 8) ? "\n-------+-------+-------\n" : "\n");
        }
        return os;
    }
};

int main(){
    sudoku_cell s;
    s.cells = {{
            #include "problem.txt"
    }};

    std::cout << s << std::endl;
    s.solve();
    std::cout << s << std::endl;
}
