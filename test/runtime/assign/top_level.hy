import "io"

let a = 3
io.println(a) // expect: 3

let b = "hello"
io.println(b) // expect: hello

let c = false
io.println(c) // expect: false

let d = a
io.println(d) // expect: 3

b = a
io.println(b) // expect: 3

d = true
io.println(d) // expect: true

let word = "testing"
io.println(word) // expect: testing

word = "another string"
io.println(word) // expect: another string

word = "yet another"
io.println(word) // expect: yet another

word = false
io.println(word) // expect: false

word = 3
io.println(word) // expect: 3
