import "io"

io.println("testing") // expect: testing

io.println("something") // io.println("another")
// expect: something

// all on its own
io.println("after") // expect: after

// io.println("starting at start of line")

io.println("after everything") // expect: after everything
