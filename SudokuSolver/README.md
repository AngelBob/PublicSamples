# Sudoku Solver

An incomplete sudoku solver.

Does okay, but still has not implemented the medium and advanced solving techniques.

## Supported Solving Methods

Pointing Pair/Locked Candidate: value must appear at the intersection of a particular box and row or column. The value can be removed as a candidate from the other cells in the box and row or column.

Naked Pair: two cells in a box, row, or column each contain the same two candidates and only those candidates. Those two values can be removed from the other cells in the box, row, or column.

Hidden Pair: Two cells in a box, row, or column have two matching candidates that do not appear in any other cell in the row, column, or box.  If the two cells contain other candidate values, those values can be removed from those cells.

## Currently Unsupported Solving Methods

These are more tricky. Not only to code up but to find reasonable testing grids to ensure they work properly.

Naked Triplet.

Hidden Triplet.

X-Wing.

XY-Wing.

Swordfish.