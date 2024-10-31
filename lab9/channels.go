//introduz canais na aplicação Go
package main

import (
	"fmt"
)

func tarefa(str chan string) {
	//escreve no canal
    msg := <-str
	fmt.Println(msg)
    str <- "Oi Main, bom dia, tudo bem?"
    msg = <-str
    fmt.Println(msg)
    str <- "Certo, entendido."
    fmt.Println("finalizado")
    str <-""
}

func main() {
	str := make(chan string)
	go tarefa(str)
    str <- "Olá Goroutine, bom dia!"
    msg := <-str
    fmt.Println(msg)
    str <- "Tudo bem! Vou terminar tá?"
    msg = <-str
    fmt.Println(msg)
    <-str
    fmt.Println("finalizado")
}
