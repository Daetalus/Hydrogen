// hello world|hello test|goodbye world|hello world how are you

import "err"

let a = "hello "
let b = "world"

err.println(a .. b)
err.println(a .. "test")
err.println("goodbye " .. b)
err.println(a .. b .. " how are " .. "you")
