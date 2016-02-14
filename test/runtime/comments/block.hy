import "io"

io.println(/* inside */ "test" /*another*/) // expect: test

/**/ io.println("empty" /**/) // expect: empty

io.println("inside /* something */ string") // expect: inside /* something */ string
/*
io.println("another")
multi-
line
	comment
*/

io.println("after") // expect: after

/*
first
io.println("test")
/*
nested
io.println("nested")
*/
io.println("after nested")
*/
io.println("after whole") // expect: after whole

// /*
io.println("starting in comment") // expect: starting in comment

// /*
// */
// */
io.println("multiple randoms") // expect: multiple randoms
