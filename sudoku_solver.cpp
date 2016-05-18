#include <iostream>
#include <iomanip>
#include <array>
#include <bitset>
#include <fstream>

struct sudoku_cell{
    using cand = std::bitset<9>;
    sudoku_cell():cells{}{}
    std::array<unsigned,81> cells;
    std::array<cand,81> candidates;
    std::array<cand,9> reduced_row;
    std::array<cand,9> reduced_column;

    cand cand_in_row(const unsigned index){
        cand ret;
        ret.set();
        const unsigned row = index/9;
        for(unsigned i=row*9;i<row*9+9;++i) if(cells[i] != 0) ret.reset(cells[i]-1);
        return ret;
    }

    cand cand_in_column(const unsigned index){
        cand ret;
        ret.set();
        const unsigned column = index%9;
        for(unsigned i=column;i<81;i+=9) if(cells[i] != 0) ret.reset(cells[i]-1);
        return ret;
    }

    cand cand_in_3x3(const unsigned index){
        cand ret;
        ret.set();
        const unsigned row = index/9;
        const unsigned column = index%9;
        const unsigned row_begin = row/3*3;
        const unsigned column_begin = column/3*3;
        for(unsigned i=row_begin;i<row_begin+3;++i){
            for(unsigned j=column_begin;j<column_begin+3;++j){
                if(cells[9*i+j] != 0) ret.reset(cells[9*i+j]-1);
            }
        }

        return ret;
    }

    inline unsigned num_of_onbit(const cand& e){ // return least canding number indicated in e
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
                if(candidates[i].count() == 1){
                    cells[i] = num_of_onbit(candidates[i]);
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

    bool solve_each_3x3(const unsigned row, const unsigned column){
        bool changed = false;
        for(unsigned i=0;i<9;++i){
            bool found = false;
            unsigned index = 0;
            for(unsigned j=3*row;j<3*row+3;++j){
                for(unsigned k=3*column;k<3*column+3;++k){
                    auto index_ = 9*j+k;
                    if(candidates[index_][i]){
                        if(found) goto LEND;
                        found = true;
                        index = index_;
                    }
                }
            }
            if(cells[index] == 0 && found){
                cells[index] = i+1;
                rebuild_cand(index);
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

    bool reduce_line_3x3(const unsigned row, const unsigned column){
        bool changed = false;
        for(unsigned i=0;i<9;++i){
            for(unsigned j=3*row;j<3*row+3;++j){
                auto index = 9*j+3*column;
                if(
                    !reduced_row[j][i] &&
                    (
                        (candidates[index][i] && (candidates[index+1][i] || candidates[index+2][i])) || (candidates[index+1][i] && candidates[index+2][i])
                    )
                ){
                    for(unsigned k=3*row;k<3*row+3;++k){
                        if(j != k){
                            for(unsigned l=3*column;l<3*column+3;++l){
                                if(candidates[9*k+l][i]) goto LEND;
                            }
                        }
                    }
                    for(unsigned k=9*j;k<9*j+9;++k){
                        if(k < 9*j+3*column || k >= 9*j+3*column+3) candidates[k].reset(i);
                    }
                    reduced_row[j].set(i);
                    changed = true;
                    goto LEND;
                }
            }

            for(unsigned j=3*column;j<3*column+3;++j){
                auto index = 27*row+j;
                if(
                    !reduced_column[j][i] &&
                    (
                        (candidates[index][i] && (candidates[index+9][i] || candidates[index+18][i])) || (candidates[index+9][i] && candidates[index+18][i])
                    )
                ){
                    for(unsigned k=3*row;k<3*row+3;++k){
                        for(unsigned l=column*3;l<column*3+3;++l){
                            if(j == l) continue;
                            if(candidates[9*k+l][i]) goto LEND;
                        }
                    }
                    for(unsigned k=j%9;k<81;k+=9){
                        if(k < j || k > j+18) candidates[k].reset(i);
                    }
                    reduced_column[j].set(i);
                    changed = true;
                    goto LEND;
                }
            }

            LEND: ;
        }
        return changed;
    }

    cand gen_cand(unsigned index){
        return (cells[index] != 0 ? cand().reset() : (cand_in_row(index) & cand_in_column(index) & cand_in_3x3(index))); 
    }

    void gen_cands(){
        for(unsigned i=0; i<81;++i) candidates[i] = gen_cand(i);
    }

    void rebuild_cand(unsigned index){
        const unsigned val = cells[index];
        const unsigned row = index/9;
        const unsigned column = index%9;
        const unsigned row_begin = row/3*3;
        const unsigned column_begin = column/3*3;
        candidates[index].reset();
        for(unsigned i=row*9;i<row*9+9;++i) candidates[i].reset(val-1);
        for(unsigned i=column;i<81;i+=9)    candidates[i].reset(val-1);
        for(unsigned i=row_begin;i<row_begin+3;++i){
            for(unsigned j=column_begin;j<column_begin+3;++j){
                candidates[9*i+j].reset(val-1);
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
