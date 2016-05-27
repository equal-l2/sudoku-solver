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
        const unsigned row = idx_to_row(idx);
        for(unsigned i=0;i<9;++i) if(cells[rc_to_idx(row,i)] != 0) ret.reset(cells[rc_to_idx(row,i)]-1);
        return ret;
    }

    numset cand_in_col(const unsigned idx){
        numset ret;
        ret.set();
        const unsigned col = idx_to_col(idx);
        for(unsigned i=0;i<9;++i) if(cells[rc_to_idx(i,col)] != 0) ret.reset(cells[rc_to_idx(i,col)]-1);
        return ret;
    }

    numset cand_in_3x3(const unsigned idx){
        numset ret;
        ret.set();
        const unsigned row_begin = idx_to_row(idx)/3*3;
        const unsigned col_begin = idx_to_col(idx)/3*3;
        for(unsigned i=row_begin;i<row_begin+3;++i){
            for(unsigned j=col_begin;j<col_begin+3;++j){
                if(cells[rc_to_idx(i,j)] != 0) ret.reset(cells[rc_to_idx(i,j)]-1);
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
            if(reduce_nearby_pair())    goto BEGIN;
            if(solve_3x3())             goto BEGIN;
            if(solve_one_cand())        goto BEGIN;
    }

    bool solve_one_cand(){
    	// solve cells which have only one candidate
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
    	// solve cells in 3x3 block
        bool changed = false;
        for(unsigned i=0;i<3;++i){
            for(unsigned j=0;j<3;++j){
                if(solve_each_3x3(i,j)) changed = true;
            }
        }
        return changed;
    }

    bool solve_each_3x3(const unsigned row_3x3, const unsigned col_3x3){
        bool changed = false;
        for(unsigned i=0;i<9;++i){
            bool found = false;
            unsigned idx = 0;
            for(unsigned j=3*row_3x3;j<3*row_3x3+3;++j){
                for(unsigned k=3*col_3x3;k<3*col_3x3+3;++k){
                    auto idx_ = rc_to_idx(j,k);
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

    bool reduce_nearby_pair(){
        bool changed = false;
        for(unsigned i=0;i<3;++i){
            for(unsigned j=0;j<3;++j){
                if(reduce_nearby_pair_3x3(i,j)) changed = true;
            }
        }
        return changed;
    }

    bool reduce_nearby_pair_3x3(const unsigned row_3x3, const unsigned col_3x3){
        bool changed = false;
        for(unsigned i=0;i<9;++i){
            for(unsigned j=3*row_3x3;j<3*row_3x3+3;++j){
                if(
                    !reduced_row[j][i] &&
                    (
                        (cands[rc_to_idx(j,3*col_3x3)][i] && (cands[rc_to_idx(j,3*col_3x3+1)][i] || cands[rc_to_idx(j,3*col_3x3+2)][i])) 
                        ||
                        (cands[rc_to_idx(j,3*col_3x3+1)][i] && cands[rc_to_idx(j,3*col_3x3+2)][i])
                    )
                ){
                    for(unsigned k=3*row_3x3;k<3*row_3x3+3;++k){
                        if(j != k){
                            for(unsigned l=3*col_3x3;l<3*col_3x3+3;++l){
                                if(cands[rc_to_idx(k,l)][i]) goto LEND;
                            }
                        }
                    }
                    for(unsigned k=0;k<9;++k){
                        if(k/3*3 != 3*col_3x3) cands[rc_to_idx(j,k)].reset(i);
                    }
                    reduced_row[j].set(i);
                    changed = true;
                    goto LEND;
                }
            }

            for(unsigned j=3*col_3x3;j<3*col_3x3+3;++j){
                if(
                    !reduced_col[j][i] &&
                    (
                        (cands[rc_to_idx(3*row_3x3,j)][i] && (cands[rc_to_idx(3*row_3x3+1,j)][i] || cands[rc_to_idx(3*row_3x3+2,j)][i])) 
                        ||
                        (cands[rc_to_idx(3*row_3x3+1,j)][i] && cands[rc_to_idx(3*row_3x3+2,j)][i])                    )
                ){
                    for(unsigned k=3*row_3x3;k<3*row_3x3+3;++k){
                        for(unsigned l=col_3x3*3;l<col_3x3*3+3;++l){
                            if(j == l) continue;
                            if(cands[rc_to_idx(k,l)][i]) goto LEND;
                        }
                    }
                    for(unsigned k=0;k<9;++k){
                        if(k/3*3 != 3*row_3x3) cands[rc_to_idx(k,j)].reset(i);
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

    void gen_cands(){
    	// generate candidates for each cells
        for(unsigned i=0; i<81;++i) if(cells[i] == 0) cands[i] = (cand_in_row(i) & cand_in_col(i) & cand_in_3x3(i));
    }

    void rebuild_cand(unsigned idx){
    	// modify candidates when value of any cells has changed
        const unsigned val = cells[idx];
        const unsigned row = idx_to_row(idx);
        const unsigned col = idx_to_col(idx);
        const unsigned row_begin = row/3*3;
        const unsigned col_begin = col/3*3;
        cands[idx].reset();
        for(unsigned i=0;i<9;++i){
            cands[rc_to_idx(row,i)].reset(val-1);
            cands[rc_to_idx(i,col)].reset(val-1);
        }
        for(unsigned i=row_begin;i<row_begin+3;++i){
            for(unsigned j=col_begin;j<col_begin+3;++j){
                cands[rc_to_idx(i,j)].reset(val-1);
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
