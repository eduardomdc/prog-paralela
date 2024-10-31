package main

import (
	"fmt"
    "math"
)

func isprime(n int) int {
    if (n<=1){return 0}
    if (n==2){return 1}
    if (n%2==0){return 0}
    for i:=3; i<int(math.Sqrt(float64(n))+1.0); i+=2{
        if(n%i == 0){return 0}
    }
    return 1
}

func sieve(job chan int, result chan bool){
    for {
        number := <- job
        is := isprime(number)
        if is == 1{
            result <- true
        } else {
            result <- false
        }
    }
} 

func main() {
    M := 5
    N := 100000000
   
    jobs := make(chan int, M)
    results := make(chan bool, M)
    for i:=0; i<M; i++{
        go sieve(jobs, results)
    }
    primes := 0
    for i:=1; i <= N; i++ {
        jobs <- i
        isprime := <-results
        if isprime {
            primes++
        }
    }
    fmt.Println(primes)
}
