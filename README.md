

The goal of this project is to produce *functionally equivalent* implementations of an example application (an OBJ file reader) using different error handling approaches for the purpose of comparing

one using exceptions and the other using return codes to perform error handling.

To allow for a fair comparison, *functionally equivalent* means in particular that the errors


## Findings

lack of exceptions brings severe restrictions:
	no way for constructors to fail
		cannot initialize container members with data in a constructor
	cannot use standard containers
	no way to define copy constructors or copy assignment operators for containers (allocation might fail, no way to return error codes)
