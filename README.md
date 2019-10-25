

The goal of this project is to produce *functionally equivalent* implementations of an example application (an OBJ file reader) using different error handling approaches for the purpose of comparing

one using exceptions and the other using return codes to perform error handling.

To allow for a fair comparison, *functionally equivalent* means in particular that the errors

OBJ loader because
	* somewhat non-trivial problem
	* somewhat performance-sensitive
	* many different kinds of errors to handle
		* I/O errors
		* syntax errors
		* allocation errors


## Findings

lack of exceptions brings severe restrictions:
	no way for constructors to fail
		cannot initialize container members with data in a constructor
	cannot use standard containers
	no way to define copy constructors or copy assignment operators for containers (allocation might fail, no way to return error codes)
	have to translate error codes at boundaries
		information might be lost (unless you make every outer error code able to represent every inner kind of error)
		exceptions do not have this problem
