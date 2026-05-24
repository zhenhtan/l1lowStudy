"""Python 面试重点 02: 面向对象与魔术方法"""


class BankAccount:
    def __init__(self, owner: str, balance: float = 0.0) -> None:
        self.owner = owner
        self.balance = balance

    def deposit(self, amount: float) -> None:
        if amount <= 0:
            raise ValueError("amount must be positive")
        self.balance += amount

    def withdraw(self, amount: float) -> None:
        if amount > self.balance:
            raise ValueError("insufficient balance")
        self.balance -= amount

    def __repr__(self) -> str:
        # __repr__ 常被问: 用于调试可读表达
        return f"BankAccount(owner={self.owner}, balance={self.balance})"


def oop_demo() -> None:
    acc = BankAccount("Alice", 100)
    acc.deposit(50)
    acc.withdraw(30)
    print(acc)


if __name__ == "__main__":
    oop_demo()
