
Optimisations
=============

* Collapse if and while loops with a constant expression that can be evaluated during compile time.
* Collapse constant expressions during compile time.
* Collapse constant immutable variables
* Collapse subsequent store instructions to the same variable with no access in between
* Binary search for fields and methods on classes, rather than linear ones. Sort by alphabetical order
