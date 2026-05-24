from robot.api.deco import keyword


@keyword("Add Numbers")
def add_numbers(a, b):
    return int(a) + int(b)


@keyword("Join With Dash")
def join_with_dash(left, right):
    return f"{left}-{right}"
