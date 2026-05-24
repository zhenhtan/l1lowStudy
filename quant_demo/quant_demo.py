"""
quant_demo.py — 最简单的 A股/港股量化交易接口示例
数据源: akshare（免费、无需注册）
演示内容:
  1. 获取历史 K 线数据
  2. 计算双均线（MA5 / MA20）
  3. 生成买卖信号
  4. 简单回测，统计收益
"""

import os
import numpy as np
import pandas as pd
import yfinance as yf


# ─────────────────────────────────────────────
# 1. 获取历史日 K 线
#    数据源：yfinance（雅虎财经，走企业代理均可）
#    A股代码映射：000001 → 000001.SS，600036 → 600036.SS
#    港股：0700.HK   美股：AAPL
#    若网络完全不通，自动生成合成数据用于演示
# ─────────────────────────────────────────────
def _make_synthetic(start: str, end: str) -> pd.DataFrame:
    """生成随机游走合成价格，用于离线演示。"""
    print("    ⚠ 网络不通，使用合成数据演示回测逻辑")
    rng = np.random.default_rng(42)
    dates = pd.bdate_range(start=start, end=end)
    n = len(dates)
    price = 30.0 * np.exp(np.cumsum(rng.normal(0, 0.015, n)))
    volume = rng.integers(1_000_000, 5_000_000, n)
    df = pd.DataFrame({
        "open":   price * (1 + rng.uniform(-0.005, 0.005, n)),
        "close":  price,
        "high":   price * (1 + rng.uniform(0.002, 0.015, n)),
        "low":    price * (1 - rng.uniform(0.002, 0.015, n)),
        "volume": volume,
    }, index=dates)
    df.index.name = "date"
    return df


def _yf_symbol(symbol: str) -> str:
    """将常见 A 股/港股代码转换为 yfinance 格式。"""
    if symbol.isdigit():
        prefix = symbol[0]
        suffix = ".SS" if prefix in ("6", "9") else ".SZ"
        return symbol + suffix
    return symbol   # 已带后缀或美股直接返回


def get_daily_data(symbol: str = "600036",   # 招商银行
                   start: str = "20230101",
                   end: str = "20241231") -> pd.DataFrame:
    """
    返回包含 open/close/high/low/volume 的 DataFrame，index 为日期。
    若 yfinance 请求失败，自动回退到合成数据。
    """
    try:
        ticker = _yf_symbol(symbol)
        df = yf.download(
            ticker,
            start=pd.Timestamp(start),
            end=pd.Timestamp(end) + pd.Timedelta(days=1),
            auto_adjust=True,
            progress=False,
        )
        if df.empty:
            raise ValueError(f"yfinance returned empty data for {ticker}")

        print(f"    成功获取 {len(df)} 行数据，日期范围 {df.index[0].date()} ~ {df.index[-1].date()}")
        print(f"    数据列：{', '.join(df.columns)}")
        # yfinance 返回多级列时展平
        if isinstance(df.columns, pd.MultiIndex):
            df.columns = [c[0].lower() for c in df.columns]
        else:
            df.columns = [c.lower() for c in df.columns]

        df.index.name = "date"
        df = df.sort_index()
        return df[["open", "close", "high", "low", "volume"]]

    except Exception as e:
        print(f"    yfinance 请求失败（{e}），切换合成数据")
        return _make_synthetic(start, end)


# ─────────────────────────────────────────────
# 2. 计算双均线信号
# ─────────────────────────────────────────────
def add_signals(df: pd.DataFrame,
                short_window: int = 5,
                long_window: int = 20) -> pd.DataFrame:
    """
    MA5 上穿 MA20 → 买入信号（signal = 1）
    MA5 下穿 MA20 → 卖出信号（signal = -1）
    其他       → 持仓不变（signal = 0）
    """
    df = df.copy()
    df["ma_short"] = df["close"].rolling(short_window).mean()
    df["ma_long"] = df["close"].rolling(long_window).mean()

    # 当前位置差（正=短线在长线上方）
    df["diff"] = df["ma_short"] - df["ma_long"]
    df["prev_diff"] = df["diff"].shift(1)

    df["signal"] = 0
    print("signal:")
    print(df["signal"])  # 统计信号分布

    print("    计算均线交叉信号...")
    df.loc[(df["diff"] > 0) & (df["prev_diff"] <= 0), "signal"] = 1   # 金叉
    df.loc[(df["diff"] < 0) & (df["prev_diff"] >= 0), "signal"] = -1  # 死叉
    print("    共生成 %d 个买入信号，%d 个卖出信号" %
           (df["signal"].eq(1).sum(), df["signal"].eq(-1).sum()))
    print("    信号分布：\n%s" % df["signal"].value_counts().to_string())  # 统计信号分布
    return df


# ─────────────────────────────────────────────
# 3. 简单回测
# ─────────────────────────────────────────────
def backtest(df: pd.DataFrame, init_cash: float = 100_000.0):
    """
    每次金叉全仓买入，死叉全仓卖出，不考虑手续费。
    返回最终资产和交易记录。
    """
    cash = init_cash
    shares = 0
    trades = []

    for date, row in df.iterrows():
        price = row["close"]

        if row["signal"] == 1 and cash > 0:           # 买入
            shares = int(cash // (price * 100)) * 100  # 整手
            cost = shares * price
            if shares > 0:
                cash -= cost
                trades.append({"date": date, "action": "BUY",
                                "price": price, "shares": shares, "cash": cash})

        elif row["signal"] == -1 and shares > 0:       # 卖出
            cash += shares * price
            trades.append({"date": date, "action": "SELL",
                            "price": price, "shares": shares, "cash": cash})
            shares = 0

    # 最后按收盘价清仓估值
    final_price = df["close"].iloc[-1]
    total = cash + shares * final_price

    return total, pd.DataFrame(trades)


# ─────────────────────────────────────────────
# 4. 主流程
# ─────────────────────────────────────────────
def main():
    SYMBOL = "002637"     # 招商银行；A股: "000001" 平安银行 / 港股: "0700.HK" / 美股: "AAPL"
    START = "20260406"
    END = "20260507"
    INIT_CASH = 100_000.0

    print(f"[1] 获取 {SYMBOL} 日K数据 {START}~{END} ...")
    df = get_daily_data(SYMBOL, START, END)
    print(f"    共 {len(df)} 个交易日\n")
    print("    所有数据预览：")
    print(df.to_string(), "\n")
    print("    数据描述统计：")
    print(df.describe().to_string(), "\n")
    print("    数据时间范围：")
    print(f"    从 {df.index[0].date()} 到 {df.index[-1].date()}\n")
    print("    数据列信息：")
    for col in df.columns:
        print(f"    - {col}: dtype={df[col].dtype}, "
              f"non-null={df[col].notnull().sum()}, "
              f"unique={df[col].nunique()}")

    print("[2] 计算双均线信号（MA5 / MA20）...")
    df = add_signals(df)
    signals = df[df["signal"] != 0][["close", "ma_short", "ma_long", "signal"]]
    print(f"    共产生 {len(signals)} 个信号：")
    print(signals.to_string(), "\n")

    print("[3] 简单回测...")
    final_value, trades = backtest(df, INIT_CASH)
    print(f"    初始资金：{INIT_CASH:,.2f}")
    print(f"    最终市值：{final_value:,.2f}")
    pct = (final_value - INIT_CASH) / INIT_CASH * 100
    print(f"    总收益率：{pct:+.2f}%\n")

    if not trades.empty:
        print("[4] 交易记录：")
        pd.set_option("display.max_rows", None)
        print(trades.to_string(index=False))
    else:
        print("[4] 没有产生交易（均线窗口期内无交叉）")


if __name__ == "__main__":
    main()
