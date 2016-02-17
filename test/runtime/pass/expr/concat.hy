import "io"

let a = "hello "
let b = "world"

io.println(a .. b) // expect: hello world
io.println(a .. "test") // expect: hello test
io.println("goodbye " .. b) // expect: goodbye world
io.println(a .. b .. " how are " .. "you") // expect: hello world how are you
io.println(a .. "world " .. b) // expect: hello world world
io.println("this " .. a .. "is " .. b .. " test ") // expect: this hello is world test
