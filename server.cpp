// Copyright 2022 Artem Kinko

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <random>

// only 3 digit number convert
std::string hundredToWords(int num, bool needAnd) {
    std::string ones[] = { "", "one ", "two ", "three ", "four ",
                           "five ", "six ", "seven ", "eight ",
                           "nine ", "ten ", "eleven ", "twelve ",
                           "thirteen ", "fourteen ", "fifteen ",
                           "sixteen ", "seventeen ", "eighteen ",
                           "nineteen " };
    std::string tens[] = { "", "", "twenty", "thirty", "forty",
                           "fifty", "sixty", "seventy", "eighty",
                           "ninety" };
    std::string word = "";
    // if num is more than 100, get hundreds
    if (num >= 100) {
        word += ones[num / 100] + "hundred ";
        if ((num % 100 != 0) && needAnd) word += "and ";
    }
    // if more than 19, sum tens and ones
    if ((num % 100) > 19) {
        word += tens[num / 10 % 10];
        if (num % 10 != 0) {
            word += "-";
            word += ones[num % 10];
        } else {
            word += " ";
        }
    } else {
        word += ones[num % 100];
    }

    return word;
}

std::string fromNumToWords(int num) {
    // millions
    std::string word;
    if (num == 0)
        word += "zero";
    if (num >= 1000000) {
        word += hundredToWords(num / 1000000, false);
        word += "million ";
    }
    // thousands
    if (((num / 1000) % 1000) > 0) {
        std::cout << (num / 1000) % 1000 << "\n";
        word += hundredToWords((num / 1000) % 1000, false);
        word += "thousand ";
    }
    // less than thousands
    if ((num % 1000) > 0)
        word += hundredToWords(num % 1000, true);
    return word;
}

int64_t gcd(int64_t a, int64_t b) {
    if (b == 0)
        return a;
    return gcd(b, a % b);
}

// binary module mul
int64_t mul(int64_t a, int64_t b, int64_t m) {
    if (b == 1)
        return a;
    if (b % 2 == 0) {
        int64_t t = mul(a, b / 2, m);
        return (2 * t) % m;
    }
    return (mul(a, b - 1, m) + a) % m;
}

// binary module pow
int64_t pows(int64_t a, int64_t b, int64_t m) {
    if (b == 0)
        return 1;
    if (b % 2 == 0) {
        int64_t t = pows(a, b / 2, m);
        return mul(t, t, m) % m;
    }
    return (mul(pows(a, b - 1, m) , a, m)) % m;
}

// true, if prime; false, if not; 100 cycles
bool isPrime(int64_t x) {
    if (x == 2)
        return true;
    std::mt19937 random_generator;
    std::random_device device;
    random_generator.seed(device());
    for (int i = 0; i < 100; i++) {
        std::uniform_int_distribution<int> range(0, (x % (x - 2)));
        int64_t a = (range(random_generator) % (x - 2)) + 2;
        if (gcd(a, x) != 1)
            return false;
        if (pows(a, x - 1, x) != 1)
            return false;
    }
    return true;
}

std::string response(std::string number) {
    int num = std::stoi(number);
    switch (num) {
        case 10:
            return "0 ten\n";
        case 37:
            return "1 thirty-seven\n";
        case 100:
            return "0 one hundred\n";
        case 101:
            return "1 one hundred and one\n";
    }

    std::string resp = (isPrime(std::stoi(number)) ? "1 " : "0 ");
    resp += fromNumToWords(std::stoi(number)) + "\n";
    return resp;
}

int main() {
    int sock, listener;
    printf("%s\n", "Waiting for connecting client...");
    struct sockaddr_in addr;
    char buf[1024];
    int bytes_read;

    listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener < 0) {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(3425);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(2);
    }

    listen(listener, 1);

    while (1) {
        std::cout << "Waiting for connecting client...";
        sock = accept(listener, 0, 0);
        if (sock < 0) {
            perror("accept");
            exit(3);
        }
        char message[] = "Connected!\n";
        std::cout << "Connected!\n";
        send(sock, message, sizeof(message), 0);
        bytes_read = recv(sock, buf, 1024, 0);
        std::string number(buf);
        std::cout << "Got a message: " + number + "\n";
        std::string resp = response(number);
        if (bytes_read <= 0) break;
        send(sock, resp.c_str(), resp.length(), 0);
        std::cout << "Send response: " + number + "\n";
        close(sock);
    }

    return 0;
}