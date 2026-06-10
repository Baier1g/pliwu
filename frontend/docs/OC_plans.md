# Plans for OC
Though the bachelor project has been finished, OC itself is not in a satisfying state. It has basic functionality we want to add, errors that need to be fixed and features we want to expand the language with. This document will attempt to organize these in an easier way, as well as allow for checking off features and fixes when these have been implemented.

## Functionality
This section will hold a list of the functionalities to be implemented.
- Modulo operator (%)
- Bitshifting (>> and <<)
- Floating point numbers
- Bitwise logical operators (& and |)
- Unary operator (- and !)
- Pre- and postfix arithmetic operators (-- and ++)
- Arithmetic and logical assignment operators (+=, -=, *=, /=, &=, |= and %= (module maybe not))
- Postfix expression or library function for length of arrays (either length(arr) or arr.length)
- Constants and enums

## Fixes and other things lackluster
This section focuses on parts of the language that are unfinished, has errors that need to be fixed or are just generally implemented with a subpar set of features

### Arrays
Arrays are very lackluster in their implementation. While they do currently work, one is very limited in their use.
Todos for arrays:
- Arrays of strings don't work when indexing
- A subarray can't be assigned to a variable of correct dimensionality, like this:
`arr[5][5];`
`arr2[] = arr[3];`
- Arrays cannot be returned from functions
- Possibly allow for deep copies of arrays to be passed around, so the user can decide whether they want to pass by reference or by value

### Test suite maturity
The test suite for OC features is currently in its infancy. While it does work, there is not a large amount test files to run and most of the ones that are implemented with the framework lack conciseness and direction. 


## Features
This section describes features to be implemented and plans for these

### Multi-module functionality

### Proper I/O to and from files, as well as allowing for the passing of string args to programs