*** Settings ***
Documentation     最小 Robot 用例：调用 Python 自定义关键字
Library           keywords.py

*** Test Cases ***
加法计算应该正确
    ${result}=    Add Numbers    2    3
    Should Be Equal As Integers    ${result}    5

字符串拼接应该正确
    ${text}=    Join With Dash    hello    robot
    Should Be Equal    ${text}    hello-robot
