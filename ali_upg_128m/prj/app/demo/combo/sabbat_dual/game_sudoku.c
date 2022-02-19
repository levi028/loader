/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: game_sudoku.c
*
*    Description:   The realize of the game sudoku
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>

#include "game_sudoku.h"
#include "game_sudoku_templet.h"
#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <asm/chip.h>

#define HRD_PRINTF(...)
//#define HRD_PRINTF libc_printf

#define MAXX    0xffff
#define N_TEMPLATE  10//(sizeof(templet)/81) //temporarily only use 10 template,john
#define TITLE_LINE    4
#define TOP           6
#define LEFT         27
#define BOTTOM       (TOP+3*4)
#define RIGHT        (LEFT+3*8)
#define STATUS_LINE  20
#define FILE_LINE    21
#define LATTICE_MAX (9*9)
#define LATTICE_SIDE 9
#define COUNT_NUM 2

UINT8 pboard[LATTICE_MAX];

/* For a general board it may be necessary to do backtracking (i.e. to
 * rewind the board to an earlier state), and make choices during the
 * solution process. This can be implemented naturally using recursion,
 * but it is more efficient to maintain a single board.
 */
int board[ LATTICE_MAX ];
int solved_board[LATTICE_MAX];

/* Record move history, and maintain a counter for the current
 * move number. Concessions are made for the user interface, and
 * allow digit 0 to indicate clearing a square. The move history
 * is used to support 'undo's for the user interface, and hence
 * is larger than required - there is sufficient space to solve
 * the puzzle, undo every move, and then redo the puzzle - and
 * if the user requires more space, then the full history will be
 * lost.
 */
static int idx_history= 0;
static int history[ 3 * LATTICE_MAX ];
static UINT8 *cur_tmplt= 0;
int game_class= 0;

/* Possible moves for a given board (c.f. fillmoves()).
 * Also used by choice() when the deterministic solver has failed,
 * and for calculating user hints. The number of hints is stored
 * in num_hints, or -1 if no hints calculated. The number of hints
 * requested by the user since their last move is stored in req_hints;
 * if the user keeps requesting hints, start giving more information.
 * Finally, record the last hint issued to the user; attempt to give
 * different hints each time.
 */
static int idx_possible= 0;
static int possible[ LATTICE_MAX ];
static int pass= 0;    /* count # passes of deterministic solver */
int completed= 0;

/* Support for template file */

static int tmplt[ LATTICE_MAX ];             /* Template indices */
static int len_tmplt= 0;               /* Number of template indices */

typedef struct _tsort_data
{
  /* qsort variables */
  int (*qcmp)(const void *, const void *);  /* the comparison routine */
  int qsz;                      /* size of each record */
  int thresh;                   /* THRESHold in chars */
  int mthresh;                  /* MTHRESHold in chars */

} TSORTDATA, *PSORTDATA;

static TSORTDATA sortdata;

#define     THRESH      4       /* threshold for insertion */
#define     MTHRESH     6       /* threshold for median */

/*
 * qst:
 * Do a quicksort
 * First, find the median element, and put that one in the first place as the
 * discriminator.  (This "median" is just the median of the first, last and
 * middle elements).  (Using this median instead of the first element is a big
 * win).  Then, the usual partitioning/swapping, followed by moving the
 * discriminator into the right place.  Then, figure out the sizes of the two
 * partions, do the smaller one recursively and the larger one via a repeat of
 * this code.  Stopping when there are less than THRESH elements in a partition
 * and cleaning up with an insertion sort (in our caller) is a huge win.
 * All data swaps are done in-line, which is space-losing but time-saving.
 * (And there are only three places where this is done).
 */

static void qst(PSORTDATA ptsortdata, char *base, char *max)
{
    char c= 0;
    char *i= 0;
    char *j= 0;
    char *jj= 0;
    int ii= 0;
    char *mid= NULL;
    char *tmp= NULL;
    int lo= 0;
    int hi= 0;
    int ret= 0;

    if((!ptsortdata) || (!base) || (!max))
    {
        return;
    }
    /*
    * At the top here, lo is the number of characters of elements in the
    * current partition.  (Which should be max - base).
    * Find the median of the first, last, and middle element and make
    * that the middle element.  Set j to largest of first and middle.
    * If max is larger than that guy, then it's that guy, else compare
    * max with loser of first and take larger.  Things are set up to
    * prefer the middle, then the first in case of ties.
    */
    lo = max - base;      /* number of elements as chars */
    do
    {
        mid = base + ptsortdata->qsz * ((lo / ptsortdata->qsz) /2);
        i = mid;
        if (lo >= ptsortdata->mthresh)
        {
            j = (ptsortdata->qcmp((jj = base), i) > 0 ? jj : i);
            ret = ptsortdata->qcmp(j, (tmp = max - ptsortdata->qsz));
            if (ret > 0)
            {
                /* switch to first loser */
                j = (j == jj ? i : jj);
                if (ptsortdata->qcmp(j, tmp) < 0)
                {
                    j = tmp;
                }
            }
            if (j != i)
            {
                ii = ptsortdata->qsz;
                do
                {
                    c = *i;
                    *i = *j;
                    i++;
                    *j = c;
                    j++;
                } while (--ii);
            }
        }
        /*
        * Semi-standard quicksort partitioning/swapping
        */
        for (i = base, j = max - ptsortdata->qsz; ; )
        {
            while (i < mid && ptsortdata->qcmp(i, mid) <= 0)
            {
                i += ptsortdata->qsz;
            }
            while (j > mid)
            {
                if (ptsortdata->qcmp(mid, j) <= 0)
                {
                    j -= ptsortdata->qsz;
                    continue;
                }
                tmp = i + ptsortdata->qsz;  /* value of i after swap */
                if (i == mid)
                {
                    /* j <-> mid, new mid is j */
                    mid = j;
                    jj = j;
                }
                else
                {
                    /* i <-> j */
                    jj = j;
                    j -= ptsortdata->qsz;
                }
                goto swap;
            }
            if (i == mid)
            {
                break;
            }
            else
            {
                /* i <-> mid, new mid is i */
                jj = mid;
                tmp = i;        /* value of i after swap */
                mid = i;
                j -= ptsortdata->qsz;
            }
swap:
            ii = ptsortdata->qsz;
            do
            {
                c = *i;
                *i = *jj;
                i++;
                *jj = c;
                jj++;
            }
            while (--ii);
            i = tmp;
        }
        /*
        * Look at sizes of the two partitions, do the smaller
        * one first by recursion, then do the larger one by
        * making sure lo is its size, base and max are update
        * correctly, and branching back.  But only repeat
        * (recursively or by branching) if the partition is
        * of at least size THRESH.
        */
        i = (j = mid) + ptsortdata->qsz;
        if ((lo = j - base) <= (hi = max - i))
        {
            if (lo >= ptsortdata->thresh)
            {
                qst(ptsortdata, base, j);
            }
            base = i;
            lo = hi;
        }
        else
        {
            if (hi >= ptsortdata->thresh)
            {
                qst(ptsortdata, i, max);
            }
            max = j;
        }
    } while (lo >= ptsortdata->thresh);
}

/*
 * qsort:
 * First, set up some global parameters for qst to share.  Then, quicksort
 * with qst(), and then a cleanup insertion sort ourselves.  Sound simple?
 * It's not...
 *
 * @implemented
 */
static void qsort(void *base0, int n, int size, int (*compar)(const void*, const void*))
{
    PSORTDATA ptsortdata =NULL;
    char *base = NULL;
    char c= 0;
    char *i= NULL;
    char *j= NULL;
    char *lo= NULL;
    char *hi= NULL;
    char *min= NULL;
    char *max= NULL;

    if ((n <= 1) || (!base0) || (!compar))
    {
        return;
    }
    base = (char *)base0;
    ptsortdata = &sortdata;

    ptsortdata->qsz = size;
    ptsortdata->qcmp = compar;
    ptsortdata->thresh = ptsortdata->qsz * THRESH;
    ptsortdata->mthresh = ptsortdata->qsz * MTHRESH;
    max = base + n * ptsortdata->qsz;
    if (n >= THRESH)
    {
        qst(ptsortdata, base, max);
        hi = base + ptsortdata->thresh;
    }
    else
    {
        hi = max;
    }
    /*
    * First put smallest element, which must be in the first THRESH, in
    * the first position as a sentinel.  This is done just by searching
    * the first THRESH elements (or the first n if n < THRESH), finding
    * the min, and swapping it into the first position.
    */
    for (j = base,lo = base; (lo += ptsortdata->qsz) < hi; )
    {
        if (ptsortdata->qcmp(j, lo) > 0)
        {
            j = lo;
        }
    }
    if (j != base)
    {
        /* swap j into place */
        for (i = base, hi = base + ptsortdata->qsz; i < hi; )
        {
            c = *j;
            *j = *i;
            j++;
            *i = c;
            i++;
        }
    }
    /*
    * With our sentinel in place, we now run the following hyper-fast
    * insertion sort.  For each remaining element, min, from [1] to [n-1],
    * set hi to the index of the element AFTER which this one goes.
    * Then, do the standard insertion sort shift on a character at a time
    * basis for each element in the frob.
    */
    for (min = base; (hi = min += ptsortdata->qsz) < max; )
    {
        while (ptsortdata->qcmp(hi -= ptsortdata->qsz, min) > 0)
        {
            ;
        }
        if ((hi += ptsortdata->qsz) != min)
        {
            for (lo = min + ptsortdata->qsz; --lo >= min; )
            {
                c = *lo;
                for (i = lo,j = lo; (j -= ptsortdata->qsz) >= hi; i = j)
                {
                    *i = *j;
                }
                *i = c;
            }
        }
    }
}




/* Reset global state */
static void reset( void )
{
    MEMSET( board, 0x00, sizeof( board ) );
    MEMSET( history, 0x00, sizeof( history ) );
    idx_history = 0;
    pass = 0;
}

/* Management of the move history - compression */
static void compress( int limit )
{
    int i= 0;
    int j=0;

    for( i = 0,j = 0 ; i < idx_history && j < limit ; ++i )
    {
        if( !( history[ i ] & IGNORED ) )
        {
            history[ j ] = history[ i ];
            j++;
        }
    }
    for( ; i < idx_history ; ++i )
    {
        history[ j ] = history[ i ];
        j++;
    }
    idx_history = j;
}

/* Management of the move history - adding a move */
static void add_move( int idx, int digit, int choice )
{
    int i=0;

    if( sizeof( history ) / sizeof( int ) == idx_history )
    {
        compress( LATTICE_MAX );
    }
    /* Never ignore the last move */
    history[ idx_history] = SET_INDEX( idx ) | SET_DIGIT( digit ) | choice;
    idx_history++;
    /* Ignore all previous references to idx */
    for( i = idx_history - 2 ; 0 <= i ; --i )
    {
        if( GET_INDEX( history[ i ] ) == idx )
        {
            history[ i ] |= IGNORED;
            break;
        }
    }
}

/* Iteration over rows/columns/blocks handled by specialised code.
 * Each function returns a block index - call must manage element/idx.
 */
static int idx_row( int el, int idx )      /* Index within a row */
{
    return INDEX( el, idx );
}

static int idx_column( int el, int idx )   /* Index within a column */
{
    return INDEX( idx, el );
}

static int idx_block( int el, int idx )    /* Index within a block */
{
    return INDEX( 3 * ( el / 3 ) + idx / 3, 3 * ( el % 3 ) + idx % 3 );
}

/* Update board state after setting a digit (clearing not handled)
 */
static void update( int idx )
{
    const int row = ROW( idx );
    const int col = COLUMN( idx );
    const int block = IDX_BLOCK( row, col );
    const int mask = DIGIT_STATE( DIGIT( idx ) );
    int i= 0;

    board[ idx ] |= STATE_MASK;  /* filled - no choice possible */

    /* Digit cannot appear in row, column or block */
    for( i = 0 ; i < 9 ; ++i )
    {
        board[ idx_row( row, i ) ] |= mask;
        board[ idx_column( col, i ) ] |= mask;
        board[ idx_block( block, i ) ] |= mask;
    }
}

/* Refresh board state, given move history. Note that this can yield
 * an incorrect state if the user has made errors - return -1 if an
 * incorrect state is generated; else return 0 for a correct state.
 */
static int reapply( void )
{
    int digit= 0;
    int idx= 0;
    int j= 0;
    int allok = 0;

    MEMSET( board, 0x00, sizeof( board ) );//libc_printf("history:%d\n",idx_history);
    for( j = 0 ; j < idx_history ; ++j )
    {
        if( !( history[ j ] & IGNORED ) && 0 != GET_DIGIT( history[ j ] ) )
        {
            idx = GET_INDEX( history[ j ] );
            digit = GET_DIGIT( history[ j ] );
            if( !IS_EMPTY( idx ) || DISALLOWED( idx, digit ) )
            {
                allok = -1;
            }
            board[ idx ] = SET_DIGIT( digit );
            if( history[ j ] & FIXED )
            {
                board[ idx ] |= FIXED;
            }
            update( idx );
        }
    }
    return allok;
}

/* Clear moves, leaving fixed squares
 */
static void clear_moves( void )
{
    int ret= 0;

    for( idx_history = 0 ; history[ idx_history ] & FIXED ; ++idx_history )
    {
        ;
    }
    ret = reapply( );
    if(ret)
    {
        ali_trace(&ret);
    }
}

static int digits[ 9 ];    /* # digits expressed in element square */
static int counts[ 9 ];    /* Count of digits (c.f. count_set_digits()) */

/* Count # set bits (within STATE_MASK) */
static int numset( int mask )
{
    int i= 0;
    int n = 0;

    for( i = STATE_SHIFT + 1 ; i <= STATE_SHIFT + 9 ; ++i )
    {
        if( mask & (1<<i) )
        {
            ++n;
        }
        else
        {
            ++counts[ i - STATE_SHIFT - 1 ];
        }
    }
    return n;
}

static void count_set_digits( int el, int (*idx_fn)( int, int ) )
{
    int i= 0;

    if(!idx_fn)
    {
        return;
    }
    MEMSET( counts, 0x00, sizeof( counts ) );
    for( i = 0 ; i < 9 ; ++i )
    {
        digits[ i ] = numset( board[ (*idx_fn)( el, i ) ] );
        if(!digits[i])
        {
            ali_trace(digits+i);
        }
    }
}

/* Fill square with given digit, and update state.
 * Returns 0 on success, else -1 on error (i.e. invalid fill)
 */
static int fill( int idx, int digit )
{
    ASSERT( 0 != digit );

    if( !IS_EMPTY( idx ) )
    {
        return ( DIGIT( idx ) == digit ) ? 0 : -1;
    }
    if( DISALLOWED( idx, digit ) )
    {
        return -1;
    }
    board[ idx ] = SET_DIGIT( digit );
    update( idx );
    add_move( idx, digit, 0 );

    return 0;
}


/* Find all squares with a single digit allowed -- do not mutate board */
static void singles( int el, int (*idx_fn)( int, int ), int hintcode )
{
    int i= 0;
    int j= 0;
    int idx= 0;

    if(!idx_fn)
    {
        return;
    }
    count_set_digits( el, idx_fn );

    for( i = 0 ; i < 9 ; ++i )
    {
        if( 1 == counts[ i ] )
        {
            /* Digit 'i+1' appears just once in the element */
            for( j = 0 ; j < 9 ; ++j )
            {
                idx = (*idx_fn)( el, j );
                if( !DISALLOWED( idx, i + 1 ) && idx_possible < LATTICE_MAX )
                {
                    possible[ idx_possible] = SET_INDEX( idx )
                                               | SET_DIGIT( i + 1 )
                                               | hintcode;
                     idx_possible++;
                }
            }
        }
        if( (LATTICE_SIDE-1) == digits[ i ] )
        {
            /* 8 digits are masked at this position - just one remaining */
            idx = (*idx_fn)( el, i );
            for( j = 1 ; j <= 9 ; ++j )
            {
                if( 0 == ( STATE( idx ) & DIGIT_STATE( j ) ) && idx_possible < LATTICE_MAX )
                {
                    possible[ idx_possible] = SET_INDEX( idx )
                                               | SET_DIGIT( j )
                                               | hintcode;
                    idx_possible++;
                }
            }
        }
    }
}

/* Given the board state, find all possible 'moves' (i.e. squares with just
 * a single digit).
 *
 * Returns the number of (deterministic) moves (and fills the moves array),
 * or 0 if no moves are possible. This function does not mutate the board
 * state, and hence, can return the same move multiple times (with
 * different hints).
 */
static int findmoves( void )
{
    int i= 0;

    idx_possible = 0;
    for( i = 0 ; i < 9 ; ++i )
    {
        singles( i, idx_row, HINT_ROW );
        singles( i, idx_column, HINT_COLUMN );
        singles( i, idx_block, HINT_BLOCK );
    }
    return idx_possible;
}

/* Strategies for refining the board state
 *  - 'pairs'     if there are two unfilled squares in a given row/column/
 *                block with the same state, and just two possibilities,
 *                then all other unfilled squares in the row/column/block
 *                CANNOT be either of these digits.
 *  - 'block'     if the unknown squares in a block all appear in the same
 *                row or column, then all unknown squares outside the block
 *                and in the same row/column cannot be any of the unknown
 *                squares in the block.
 *  - 'common'    if all possible locations for a digit in a block appear
 *                in a row or column, then that digit cannot appear outside
 *                the block in the same row or column.
 *  - 'position2' if the positions of 2 unknown digits in a block match
 *                identically in precisely 2 positions, then those 2 positions
 *                can only contain the 2 unknown digits.
 *
 * Recall that each state bit uses a 1 to prevent a digit from
 * filling that square.
 */

static void pairs( int el, int (*idx_fn)( int, int ) )
{
    int i= 0;
    int j= 0;
    int k= 0;
    int mask= 0;
    int idx= 0;

    if(!idx_fn)
    {
        return;
    }
    for( i = 0 ; i < 8 ; ++i )
    {
        if( (LATTICE_SIDE-2) == digits[ i ] ) /* 2 digits unknown */
        {
            for( j = i + 1 ; j < 9 ; ++j )
            {
                idx = (*idx_fn)( el, i );
                if( STATE( idx ) == STATE( (*idx_fn)( el, j ) ) )
                {
                    /* Found a row/column pair - mask other entries */
                    mask = STATE_MASK ^ (STATE_MASK & board[ idx ] );
                    for( k = 0 ; k < i ; ++k )
                    {
                        board[ (*idx_fn)( el, k ) ] |= mask;
                    }
                    for( k = i + 1 ; k < j ; ++k )
                    {
                        board[ (*idx_fn)( el, k ) ] |= mask;
                    }
                    for( k = j + 1 ; k < 9 ; ++k )
                    {
                        board[ (*idx_fn)( el, k ) ] |= mask;
                    }
                    digits[ j ] = -1; /* now processed */
                }
            }
        }
    }
}

/* Worker: mask elements outside block */
static void exmask( int mask, int block, int el, int (*idx_fn)( int, int ) )
{
    int i= 0;
    int idx= 0;

    if(!idx_fn)
    {
        return;
    }
    for( i = 0 ; i < 9 ; ++i )
    {
        idx = (*idx_fn)( el, i );
        if( block != BLOCK( idx ) && IS_EMPTY( idx ) )
        {
            board[ idx ] |= mask;
        }
    }
}

/* Worker for block() */
static void exblock( int block, int el, int (*idx_fn)( int, int ) )
{
    int i= 0;
    int idx= 0;
    int mask= 0;

    if(!idx_fn)
    {
        return;
    }
    /* By assumption, all unknown squares in the block appear in the
     * same row/column, so to construct a mask for these squares, it
     * is sufficient to invert the mask for the known squares in the
     * block.
     */
    mask = 0;
    for( i = 0 ; i < 9 ; ++i )
    {
        idx = idx_block( block, i );
        if( !IS_EMPTY( idx ) )
        {
            mask |= DIGIT_STATE( DIGIT( idx ) );
        }
    }
    exmask( mask ^ STATE_MASK, block, el, idx_fn );
}

static void block( int el )
{
    int i= 0;
    int idx = 0;
    int row= 0;
    int col= 0;

    /* Find first unknown square */
    for( i = 0 ; i < 9 && !IS_EMPTY( idx = idx_block( el, i ) ) ; ++i )
    {
        ;
    }
    if( i < LATTICE_SIDE )
    {
        ASSERT( IS_EMPTY( idx ) );
        row = ROW( idx );
        col = COLUMN( idx );
        for( ++i ; i < 9 ; ++i )
        {
            idx = idx_block( el, i );
            if( IS_EMPTY( idx ) )
            {
                if( ROW( idx ) != row )
                {
                    row = -1;
                }
                if( COLUMN( idx ) != col )
                {
                    col = -1;
                }
            }
        }
        if( 0 <= row )
        {
            exblock( el, row, idx_row );
        }
        if( 0 <= col )
        {
            exblock( el, col, idx_column );
        }
    }
}

static void common( int el )
{
    int i= 0;
    int idx= 0;
    int row= 0;
    int col= 0;
    int digit= 0;
    int mask= 0;

    for( digit = 1 ; digit <= 9 ; ++digit )
    {
        mask = DIGIT_STATE( digit );
        row = -1;  /* Value '9' indicates invalid */
        col = -1;
        for( i = 0 ; i < 9 ; ++i )
        {
            /* Digit possible? */
            idx = idx_block( el, i );
            if( IS_EMPTY( idx ) && 0 == ( board[ idx ] & mask ) )
            {
                if( row < 0 )
                {
                    row = ROW( idx );
                }
                else if( row != ROW( idx ) )
                {
                    row = 9; /* Digit appears in multiple rows */
                }
                if( col < 0 )
                {
                    col = COLUMN( idx );
                }
                else if( col != COLUMN( idx ) )
                {
                    col = 9; /* Digit appears in multiple columns */
                }
            }
        }
        if( (-1 != row) && (row < LATTICE_SIDE) )
        {
            exmask( mask, el, row, idx_row );
        }
        if( (-1 != col) && (col < LATTICE_SIDE) )
        {
            exmask( mask, el, col, idx_column );
        }
    }
}

/* Encoding of positions of a digit (c.f. position2()) - abuse DIGIT_STATE */
static int posn_digit[ 10 ];
static void conflict_manage(int digit,int digit2,int el)
{
    int mask= 0;
    int mask2= 0;
    int i= 0;
    int idx= 0;

    mask = STATE_MASK
           ^ ( DIGIT_STATE( digit ) | DIGIT_STATE( digit2 ) );
    mask2 = DIGIT_STATE( digit );
    for( i = 0 ; i < 9 ; ++i )
    {
        idx = idx_block( el, i );
        if( 0 == ( mask2 & board[ idx ] ) )
        {
            ASSERT( 0 == (DIGIT_STATE(digit2) & board[idx]) );
            board[ idx ] |= mask;
        }
    }
    posn_digit[ digit ]  = 0;
    posn_digit[ digit2 ] = 0;
}

static void position2( int el )
{
    int digit= 0;
    int digit2= 0;
    int i= 0;
    int mask= 0;
    int posn= 0;
    int count= 0;

    /* Calculate positions of each digit within block */
    for( digit = 1 ; digit <= 9 ; ++digit )
    {
        mask = DIGIT_STATE( digit );
        posn_digit[ digit ] = 0;
        count = 0;
        posn = 0;
        for( i = 0 ; i < 9 ; ++i )
        {
            if( 0 == ( mask & board[ idx_block( el, i ) ] ) )
            {
                ++count;
                posn |= DIGIT_STATE( i );
            }
        }
        if( COUNT_NUM == count )
        {
            posn_digit[ digit ] = posn;
        }
    }
    /* Find pairs of matching positions, and mask */
    for( digit = 1 ; digit < 9 ; ++digit )
    {
        if( 0 != posn_digit[ digit ] )
        {
            for( digit2 = digit + 1 ; digit2 <= 9 ; ++digit2 )
            {
                if( posn_digit[ digit ] == posn_digit[ digit2 ] )
                {
                    conflict_manage(digit, digit2, el);
                    break;
                }
            }
        }
    }
}

/* Find some moves for the board; starts with a simple approach (finding
 * singles), and if no moves found, starts using more involved strategies
 * until a move is found. The more advanced strategies can mask states
 * in the board, making this an efficient mechanism, but difficult for
 * a human to understand.
 */
static int allmoves( void )
{
    int i= 0;
    int n= 0;

    n = findmoves( );
    if( 0 < n )
    {
        return n;
    }

    for( i = 0 ; i < 9 ; ++i )
    {
        count_set_digits( i, idx_row );
        pairs( i, idx_row );

        count_set_digits( i, idx_column );
        pairs( i, idx_column );

        count_set_digits( i, idx_block );
        pairs( i, idx_block );
    }
    n = findmoves( );
    if( 0 < n )
    {
        return n;
    }
    for( i = 0 ; i < 9 ; ++i )
    {
        block( i );
        common( i );
        position2( i );
    }
    return findmoves( );
}

/* Deterministic solver; return 0 on success, else -1 on error.
 */
static int deterministic( void )
{
    int i= 0;
    int n= 0;

    n = allmoves( );
    while( 0 < n )
    {
        ++pass;
        for( i = 0 ; i < n ; ++i )
        {
            if( -1 == fill( GET_INDEX( possible[ i ] ),
                            GET_DIGIT( possible[ i ] ) ) )
            {
                return -1;
            }
        }
        n = allmoves( );
    }
    return 0;
}

/* Return index of square for choice.
 *
 * If no choice is possible (i.e. board solved or inconsistent),
 * return -1.
 *
 * The current implementation finds a square with the minimum
 * number of unknown digits (i.e. maximum # masked digits).
 */
static int cmp( const void *e1, const void *e2 )
{
    if((!e1) || (!e2))
    {
        return -1;
    }
    return GET_DIGIT( *(const int *)e2 ) - GET_DIGIT( *(const int *)e1 );
}

static int choice( void )
{
    int i= 0;
    int n= 0;

    for( n = 0,i = 0 ; i < LATTICE_MAX ; ++i )
    {
        if( IS_EMPTY( i ) )
        {
            possible[ n ] = SET_INDEX( i ) | SET_DIGIT( numset( board[ i ] ) );

            /* Inconsistency if square unknown, but nothing possible */
            if( LATTICE_SIDE == GET_DIGIT( possible[ n ] ) )
            {
                return -2;
            }
            ++n;
        }
    }

    if( 0 == n )
    {
        return -1;      /* All squares known */
    }
    qsort( possible, n, sizeof( possible[ 0 ] ), cmp );
    return GET_INDEX( possible[ 0 ] );
}

/* Choose a digit for the given square.
 * The starting digit is passed as a parameter.
 * Returns -1 if no choice possible.
 */
static int choose( int idx, int digit )
{
    for( ; digit <= 9 ; ++digit )
    {
        if( !DISALLOWED( idx, digit ) )
        {
            board[ idx ] = SET_DIGIT( digit );
            update( idx );
            add_move( idx, digit, CHOICE );
            return digit;
        }
    }
    return -1;
}

/* Backtrack to a previous choice point, and attempt to reseed
 * the search. Return -1 if no further choice possible, or
 * the index of the changed square.
 *
 * Assumes that the move history and board are valid.
 */
static int backtrack( void )
{
    int digit= 0;
    int idx= 0;
    int ret= 0;

    --idx_history;
    for( ; 0 <= idx_history ; )
    {
        if( history[ idx_history ] & CHOICE )
        {
            /* Remember the last choice, and advance */
            idx = GET_INDEX( history[ idx_history ] );
            digit = GET_DIGIT( history[ idx_history ] ) + 1;
            ret = reapply( );
            if(ret)
            {
                if( -1 != choose( idx, digit ) )
                {
                    return idx;
                }
            }
            else
            {
                break;
            }
        }
        --idx_history;
    }
    return -1;
}

/* Attempt to solve 'board'; return 0 on success else -1 on error.
 *
 * The solution process attempts to fill-in deterministically as
 * much of the board as possible. Once that is no longer possible,
 * need to choose a square to fill in.
 */
static int solve( void )
{
    int idx= 0;

    while( 1 )
    {
        if( 0 == deterministic( ) )
        {
            /* Solved, make a new choice, or rewind a previous choice */
            idx = choice( );
            if( -1 == idx )
            {
                return 0;
            }
            else if( ( idx < 0 || -1 == choose( idx, 1 ) ) && -1 == backtrack( ) )
            {
                return -1;
            }
        }
        else if( -1 == backtrack( ) )
        {
            return -1;
        }
    }
    return -1;
}

static void read_board( UINT8 *f )
{
    int i= 0;

    if(!f)
    {
        return;
    }
    len_tmplt = 0;

    for(i=0;i<LATTICE_MAX;i++)
    {
        if('*' == f[i])
        {
            tmplt[ len_tmplt] = i;
            len_tmplt++;
        }
    }
    return;
}

/* 3 a SuDoKu, given its solution.
 *
 * The classification is based on the average number of possible moves
 * for each pass of the deterministic solver - it is a rather simplistic
 * measure, but gives reasonable results. Note also that the classification
 * is based on the first solution found (but does handle the pathological
 * case of multiple solutions). Note that the average moves per pass
 * depends just on the number of squares initially set... this simplifies
 * the statistics collection immensely, requiring just the number of passes
 * to be counted.
 *
 * Return 0 on error, else a string classification.
 */

static int classify( void )
{
    int i= 0;
    int score= 0;

    pass = 0;
    clear_moves( );
    if( -1 == solve( ) )
    {
        return 0;
    }
    score = LATTICE_MAX;
    for( i = 0 ; i < LATTICE_MAX ; ++i )
    {
        if( IS_FIXED( i ) )
        {
            --score;
        }
    }
    ASSERT( LATTICE_MAX == idx_history );

    for( i = 0 ; i < LATTICE_MAX ; ++i )
    {
        if( history[ i ] & CHOICE )
        {
            score -= 5;
        }
    }
    if( 13 * pass < score )
    {
        return 0;
    }
    else if( 9 * pass < score )
    {
        return 1;
    }
    else if( 5 * pass < score )
    {
        return 2;
    }
    return 2;
}

/* exchange disjoint, identical length blocks of data */
static void exchange( int *a, int *b, int len )
{
    int i= 0;
    int tmp= 0;

    if((!a) || (!b))
    {
        return;
    }
    for( i = 0 ; i < len ; ++i )
    {
        tmp = a[ i ];
        a[ i ] = b[ i ];
        b[ i ] = tmp;
    }
}

/* rotate left */
static void rotate1_left( int *a, int len )
{
    int i= 0;
    int tmp= 0;

    if(!a)
    {
        return;
    }
    tmp = a[ 0 ];
    for( i = 1 ; i < len ; ++i )
    {
        a[ i - 1 ] = a[ i ];
    }
    a[ len - 1 ] = tmp;
}

/* rotate right */
static void rotate1_right( int *a, int len )
{
    int i= 0;
    int tmp= 0;

    if((!a) || (len < 1))
    {
        return ;
    }
    tmp = a[ len - 1 ];
    for( i = len - 1 ; 0 < i ; --i )
    {
        a[ i ] = a[ i - 1 ];
    }
    a[ 0 ] = tmp;
}

/* Generalised left rotation - there is a naturally recursive
 * solution that is best implementation using iteration.
 * Note that it is not necessary to do repeated unit rotations.
 *
 * This function is analogous to 'cutting' a 'pack of cards'.
 *
 * On entry: 0 < idx < len
 */
static void rotate( int *a, int len, int idx )
{
    int xdi = len - idx;
    int delta = idx - xdi;

    if(!a)
    {
        return;
    }
//  libc_printf("idx:%d\n",idx);
    while( 0 != delta && 0 != idx )
    {
        if( delta < 0 )
        {
            if( 1 == idx )
            {
                rotate1_left( a, len );
                idx = 0;
            }
            else
            {
                exchange( a, a + xdi, idx );
                len = xdi;
            }
        }
        else /* 0 < delta */
        {
            if( 1 == xdi )
            {
                rotate1_right( a, len );
                idx = 0;
            }
            else
            {
                exchange( a, a + idx, xdi );
                a += xdi;
                len = idx;
                idx -= xdi;
            }
        }
        xdi = len - idx;
        delta = idx - xdi;
    }
    if( 0 < idx )
    {
        exchange( a, a + idx, idx );
    }
}

/* Shuffle an array of integers */
static void shuffle( int *a, int len )
{
    int i= 0;
    int j= 0;
    int tmp= 0;

    if(!a)
    {
        return;
    }
    i = len;
    while( 1 <= i )
    {
        j = RAND(MAXX) % i;
        --i;
        tmp = a[i];
        a[ i ] = a[ j ];
        a[ j ] = tmp;
    }
}

/* Generate a SuDoKu puzzle
 *
 * The generation process selects a random template, and then attempts
 * to fill in the exposed squares to generate a board. The order of the
 * digits and of filling in the exposed squares are random.
 */

/* Select random template; sets tmplt, len_tmplt */
static void select_template( void )
{
    int i = RAND(MAXX) % N_TEMPLATE;

    cur_tmplt = (UINT8 *)templet[i];
//  libc_printf("i=%d\n",i);
    read_board( cur_tmplt );

}

static void generate( void )
{
    int digits[ 9 ];
//  int max_num=0,num = 0,s=0;;

    int i= 0;
    int ret= 0;

start:
    for( i = 0 ; i < 9 ; ++i )
    {
        digits[ i ] = i + 1;
    }
    rotate( digits, 9, 1 + RAND(MAXX) % 8 );

    shuffle( digits, 9 );

    select_template( );

//  max_num++;
    if(0 == (len_tmplt-1) )
    {
        return;
    }
    rotate( tmplt, len_tmplt, 1 + RAND(MAXX) % ( len_tmplt - 1 ) );
    shuffle( tmplt, len_tmplt );

    reset( );  /* construct a new board */

    for( i = 0 ; i < len_tmplt ; ++i )
    {
        ret = fill( tmplt[ i ], digits[ i % 9 ] );
        if(!ret)
        {
            ali_trace(&ret);
        }
    }
    if( 0 != solve( ) || idx_history < LATTICE_MAX )
    {
        goto start;
    }
    for( i = 0 ; i < len_tmplt ; ++i )
    {
        board[ tmplt[ i ] ] |= FIXED;
    }
    /* Construct fixed squares */
    for( idx_history = 0, i = 0 ; i < LATTICE_MAX ; ++i )
    {
        if( IS_FIXED( i ) )
        {
            history[ idx_history] = SET_INDEX( i )
                                     | SET_DIGIT( DIGIT( i ) )
                                     | FIXED;
            idx_history++;
        }
    }
    clear_moves( );

    if( 0 != solve( ) || idx_history < LATTICE_MAX )
    {
        goto start;
    }
    if( -1 != backtrack( ) && 0 == solve( ) )
    {
        goto start;
    }
    game_class = classify( );
    if(!game_class)
    {
        ali_trace(&game_class);
    }
    MEMCPY(solved_board,board,sizeof(board));

    clear_moves( );

/*  if(num<10000)
    {
        num++;
        libc_printf("max:%d\n",max_num);

        if(s<max_num) s= max_num;
        max_num=0;
        goto start;
    }
    else
    {

    }
    libc_printf("s = %d\n",s);
    */

}

/* load a new board - this could be a precanned board,
 * or a randomly generated board - chose between these
 * randomly -- 1 in 3 chance of loading a precanned
 * board (if they exist).
 */
void load_board( void )
{
    int i= 0;

//  libc_printf( "generating a random board... (please wait)\n" );
    generate( );
    completed = 0;

    for(i=0;i<LATTICE_MAX;i++)
    {
        pboard[i] = GET_DIGIT(board[i])+0x30;
        if('0' == pboard[i])
        {
            pboard[i]=' ';
        }
    //  libc_printf("p[i]:%d\n",pboard[i]);
    }
}

void reset_board( void )
{
    int i= 0;

    for(i=0;i<LATTICE_MAX;i++)
    {
        pboard[i] = GET_DIGIT(board[i])+0x30;
        if('0' == pboard[i])
        {
            pboard[i]=' ';
        }
    //  libc_printf("p[i]:%d\n",pboard[i]);
    }
}

void solve_board( void )
{
    int i= 0;

    for(i=0;i<LATTICE_MAX;i++)
    {
        pboard[i] = GET_DIGIT(solved_board[i])+0x30;
        //if(pboard[i]=='0')
        //    SDBBP();
    //  libc_printf("p[i]:%d %d\n",i,GET_DIGIT(solved_board[i]));
    }
}

