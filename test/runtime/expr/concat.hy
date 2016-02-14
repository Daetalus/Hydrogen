import "io"

let a = "hello "
let b = "world"

io.println(a .. b) // expect: hello world
io.println(a .. "test") // expect: hello test
io.println("goodbye " .. b) // expect: goodbye world
io.println(a .. b .. " how are " .. "you") // expect: hello world how are you
