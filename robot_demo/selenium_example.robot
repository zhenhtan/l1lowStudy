*** Settings ***
Documentation     最小 Selenium Robot 用例：打开页面并校验标题
Library           SeleniumLibrary
Suite Setup       Open Test Browser
Suite Teardown    Close Browser

*** Variables ***
${URL}            https://example.com
${BROWSER}        chrome

*** Keywords ***
Open Test Browser
    ${chrome_options}=    Evaluate    sys.modules['selenium.webdriver'].ChromeOptions()    sys, selenium.webdriver
    Call Method    ${chrome_options}    add_argument    --headless
    Call Method    ${chrome_options}    add_argument    --no-sandbox
    Call Method    ${chrome_options}    add_argument    --disable-dev-shm-usage
    Open Browser    ${URL}    ${BROWSER}    options=${chrome_options}
    Set Selenium Timeout    10 seconds
    Set Selenium Speed    0 seconds

*** Test Cases ***
页面标题应为 Example Domain
    Title Should Be    Example Domain
    Page Should Contain    This domain is for use in documentation examples without needing permission. Avoid use in operations.
