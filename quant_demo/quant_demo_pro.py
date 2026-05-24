"""
quant_demo_pro.py — 生产级量化回测框架
包含：交易成本、滑点、仓位管理、回撤控制、样本分离、性能报告
"""

import os
import numpy as np
import pandas as pd
import yfinance as yf
from datetime import datetime


# ─────────────────────────────────────────────
# 1. 配置与常量
# ─────────────────────────────────────────────
class Config:
    """交易配置参数"""
    commission = 0.0002       # 手续费 0.02%
    slippage = 0.0005        # 滑点 0.05%
    max_position = 0.9       # 最大持仓占资金比例 90%
    max_drawdown = 0.15      # 最大回撤容限 15%
    stop_loss = 0.08         # 单笔止损 8%
    max_daily_loss = 0.03    # 单日最大亏损 3%
    position_size = 0.8      # 单笔仓位占比 80%


# ─────────────────────────────────────────────
# 2. 数据获取与合成
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
    return symbol


def get_daily_data(symbol: str = "600036",
                   start: str = "20230101",
                   end: str = "20241231") -> pd.DataFrame:
    """返回包含 open/close/high/low/volume 的 DataFrame。"""
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

        # 展平多级列
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
# 3. 信号生成
# ─────────────────────────────────────────────
def add_signals(df: pd.DataFrame,
                short_window: int = 3,
                long_window: int = 10) -> pd.DataFrame:
    """计算双均线信号（金叉/死叉）"""
    df = df.copy()
    df["ma_short"] = df["close"].rolling(short_window).mean()
    df["ma_long"] = df["close"].rolling(long_window).mean()
    df["diff"] = df["ma_short"] - df["ma_long"]
    df["prev_diff"] = df["diff"].shift(1)

    df["signal"] = 0
    df.loc[(df["diff"] > 0) & (df["prev_diff"] <= 0), "signal"] = 1   # 金叉
    df.loc[(df["diff"] < 0) & (df["prev_diff"] >= 0), "signal"] = -1  # 死叉

    return df


# ─────────────────────────────────────────────
# 4. 纸面交易引擎
# ─────────────────────────────────────────────
class PaperTradingEngine:
    """仿真交易引擎"""
    
    def __init__(self, init_cash: float = 100_000.0, config: Config = None):
        self.init_cash = init_cash
        self.config = config or Config()
        
        self.cash = init_cash
        self.shares = 0
        self.entry_price = 0.0
        self.trades = []
        self.daily_pnl = []
        self.portfolio_value = [init_cash]
        self.max_portfolio_value = init_cash
        
    def apply_slippage(self, price: float, is_buy: bool) -> float:
        """应用滑点"""
        return price * (1 + self.config.slippage) if is_buy else price * (1 - self.config.slippage)
    
    def apply_commission(self, position_value: float) -> float:
        """计算手续费"""
        return position_value * self.config.commission
    
    def check_drawdown(self, current_value: float) -> bool:
        """检查最大回撤是否超限"""
        drawdown = (self.max_portfolio_value - current_value) / self.max_portfolio_value
        return drawdown <= self.config.max_drawdown
    
    def check_daily_loss(self, current_value: float, prev_value: float) -> bool:
        """检查单日亏损是否超限"""
        daily_loss = (prev_value - current_value) / prev_value
        return daily_loss <= self.config.max_daily_loss
    
    def execute_trade(self, date: pd.Timestamp, price: float, signal: int, 
                      prev_portfolio_value: float) -> dict:
        """执行买卖逻辑，返回交易记录"""
        trade = None
        
        if signal == 1 and self.shares == 0:  # 买入信号
            slippage_price = self.apply_slippage(price, True)
            position_value = self.cash * self.config.position_size
            shares = int(position_value / (slippage_price * 100)) * 100
            
            if shares > 0:
                cost = shares * slippage_price
                fee = self.apply_commission(cost)
                total_cost = cost + fee
                
                if total_cost <= self.cash:
                    self.shares = shares
                    self.entry_price = slippage_price
                    self.cash -= total_cost
                    trade = {
                        "date": date, "action": "BUY",
                        "entry_price": slippage_price, "shares": shares,
                        "cost": cost, "fee": fee, "cash": self.cash
                    }
        
        elif signal == -1 and self.shares > 0:  # 卖出信号
            slippage_price = self.apply_slippage(price, False)
            revenue = self.shares * slippage_price
            fee = self.apply_commission(revenue)
            net_revenue = revenue - fee
            
            pnl = net_revenue - (self.shares * self.entry_price)
            self.cash += net_revenue
            trade = {
                "date": date, "action": "SELL",
                "exit_price": slippage_price, "shares": self.shares,
                "revenue": revenue, "fee": fee, "pnl": pnl, "cash": self.cash
            }
            self.shares = 0
        
        # 止损检查
        elif self.shares > 0:
            current_value = self.shares * price
            entry_value = self.shares * self.entry_price
            loss_pct = (entry_value - current_value) / entry_value
            
            if loss_pct > self.config.stop_loss:
                slippage_price = self.apply_slippage(price, False)
                revenue = self.shares * slippage_price
                fee = self.apply_commission(revenue)
                net_revenue = revenue - fee
                pnl = net_revenue - entry_value
                
                self.cash += net_revenue
                trade = {
                    "date": date, "action": "STOP_LOSS",
                    "exit_price": slippage_price, "shares": self.shares,
                    "pnl": pnl, "loss_pct": loss_pct, "cash": self.cash
                }
                self.shares = 0
        
        return trade
    
    def update_portfolio(self, date: pd.Timestamp, close_price: float):
        """更新资产净值"""
        position_value = self.shares * close_price
        total_value = self.cash + position_value
        
        self.portfolio_value.append(total_value)
        if total_value > self.max_portfolio_value:
            self.max_portfolio_value = total_value
        
        return total_value


# ─────────────────────────────────────────────
# 5. 样本分割与验证
# ─────────────────────────────────────────────
def train_test_split(df: pd.DataFrame, split_ratio: float = 0.8):
    """按时间分割数据集"""
    split_idx = int(len(df) * split_ratio)
    train_df = df.iloc[:split_idx]
    test_df = df.iloc[split_idx:]
    return train_df, test_df


# ─────────────────────────────────────────────
# 6. 性能分析
# ─────────────────────────────────────────────
class PerformanceAnalyzer:
    """性能报告生成"""
    
    def __init__(self, trades: list, portfolio_values: list, init_cash: float):
        self.trades = trades
        self.portfolio_values = portfolio_values
        self.init_cash = init_cash
        
    def calculate_metrics(self) -> dict:
        """计算关键性能指标"""
        final_value = self.portfolio_values[-1]
        total_return = (final_value - self.init_cash) / self.init_cash
        
        # 最大回撤
        peak = self.init_cash
        max_dd = 0.0
        for val in self.portfolio_values:
            if val > peak:
                peak = val
            dd = (peak - val) / peak
            if dd > max_dd:
                max_dd = dd
        
        # 胜率
        if not self.trades:
            win_rate = 0.0
            avg_win = 0.0
            avg_loss = 0.0
            profit_factor = 0.0
        else:
            pnl_trades = [t for t in self.trades if "pnl" in t]
            if pnl_trades:
                wins = [t for t in pnl_trades if t["pnl"] > 0]
                win_rate = len(wins) / len(pnl_trades)
                avg_win = np.mean([t["pnl"] for t in wins]) if wins else 0.0
                losses = [t for t in pnl_trades if t["pnl"] < 0]
                avg_loss = abs(np.mean([t["pnl"] for t in losses])) if losses else 0.0
                profit_factor = sum(t["pnl"] for t in wins) / abs(sum(t["pnl"] for t in losses)) if losses else 0.0
            else:
                win_rate = avg_win = avg_loss = profit_factor = 0.0
        
        # 夏普比（简化）
        returns = np.diff(self.portfolio_values) / np.array(self.portfolio_values[:-1])
        sharpe = np.mean(returns) / (np.std(returns) + 1e-6) * np.sqrt(252)
        
        return {
            "final_value": final_value,
            "total_return": total_return,
            "max_drawdown": max_dd,
            "trade_count": len(self.trades),
            "win_rate": win_rate,
            "avg_win": avg_win,
            "avg_loss": avg_loss,
            "profit_factor": profit_factor,
            "sharpe_ratio": sharpe,
        }
    
    def print_report(self, name: str = "回测结果"):
        """打印完整报告"""
        metrics = self.calculate_metrics()
        print(f"\n{'='*60}")
        print(f"{name}")
        print(f"{'='*60}")
        print(f"初始资金:       {self.init_cash:>15,.2f}")
        print(f"最终市值:       {metrics['final_value']:>15,.2f}")
        print(f"总收益率:       {metrics['total_return']:>15.2%}")
        print(f"最大回撤:       {metrics['max_drawdown']:>15.2%}")
        print(f"交易次数:       {metrics['trade_count']:>15.0f}")
        print(f"胜率:           {metrics['win_rate']:>15.2%}")
        print(f"平均盈利:       {metrics['avg_win']:>15,.2f}")
        print(f"平均亏损:       {metrics['avg_loss']:>15,.2f}")
        print(f"盈亏比:         {metrics['profit_factor']:>15.2f}")
        print(f"夏普比:         {metrics['sharpe_ratio']:>15.2f}")
        print(f"{'='*60}\n")


# ─────────────────────────────────────────────
# 7. 滚动窗口验证（Walk-Forward Validation）
# ─────────────────────────────────────────────
def walk_forward_validation(
    df: pd.DataFrame,
    n_splits: int = 8,
    train_ratio: float = 0.7,
    init_cash: float = 100_000.0,
    config: Config = None,
    verbose: bool = False,
) -> pd.DataFrame:
    """
    将全量时间序列切成 n_splits 个滚动窗口：
      每个窗口前 train_ratio 用来「训练/确认参数」，
      后 1-train_ratio 用来「样本外测试」。
    相邻窗口依次向后滑动，互不重叠。
    返回汇总 DataFrame（每行一个窗口的测试集指标）。
    """
    config = config or Config()
    total = len(df)
    window_size = total // n_splits          # 每个窗口总长度
    test_size = int(window_size * (1 - train_ratio))  # 每个窗口测试集长度
    train_size = window_size - test_size

    print(f"\n[Walk-Forward] 总数据 {total} 天，"
          f"切 {n_splits} 个窗口，每窗口训练 {train_size} 天 / 测试 {test_size} 天")
    print(f"{'─'*72}")
    print(f"{'窗口':^4} {'训练区间':^23} {'测试区间':^23} "
          f"{'收益率':>8} {'最大回撤':>8} {'夏普':>6} {'交易数':>6} {'胜率':>7}")
    print(f"{'─'*72}")

    results = []
    for i in range(n_splits):
        start = i * window_size
        end = start + window_size
        if end > total:
            break

        train_df = df.iloc[start : start + train_size]
        test_df  = df.iloc[start + train_size : end]

        if len(test_df) < 5:  # 测试窗口太短跳过
            continue

        engine = PaperTradingEngine(init_cash, config)
        trades = backtest(test_df, engine)

        analyzer = PerformanceAnalyzer(trades, engine.portfolio_value, init_cash)
        m = analyzer.calculate_metrics()

        row = {
            "window": i + 1,
            "train_start": train_df.index[0].date(),
            "train_end":   train_df.index[-1].date(),
            "test_start":  test_df.index[0].date(),
            "test_end":    test_df.index[-1].date(),
            "return":      m["total_return"],
            "max_drawdown": m["max_drawdown"],
            "sharpe":      m["sharpe_ratio"],
            "trades":      m["trade_count"],
            "win_rate":    m["win_rate"],
        }
        results.append(row)

        sign = "+" if m["total_return"] >= 0 else ""
        print(f"  {i+1:2d}   {str(train_df.index[0].date()):>10}~{str(train_df.index[-1].date()):<10}  "
              f"{str(test_df.index[0].date()):>10}~{str(test_df.index[-1].date()):<10}  "
              f"{sign}{m['total_return']:>6.2%}  "
              f"{m['max_drawdown']:>7.2%}  "
              f"{m['sharpe_ratio']:>5.2f}  "
              f"{m['trade_count']:>5.0f}  "
              f"{m['win_rate']:>6.2%}")

    summary = pd.DataFrame(results)
    if summary.empty:
        return summary

    print(f"{'─'*72}")
    positive_windows = (summary["return"] > 0).sum()
    print(f"汇总 | 正收益窗口: {positive_windows}/{len(summary)}  "
          f"平均收益: {summary['return'].mean():+.2%}  "
          f"平均最大回撤: {summary['max_drawdown'].mean():.2%}  "
          f"平均夏普: {summary['sharpe'].mean():.2f}")
    print(f"{'─'*72}")
    return summary


# ─────────────────────────────────────────────
# 8. 主回测流程
# ─────────────────────────────────────────────
def backtest(df: pd.DataFrame, engine: PaperTradingEngine) -> list:
    """执行回测"""
    trades = []
    prev_portfolio = engine.init_cash
    
    for date, row in df.iterrows():
        price = row["close"]
        signal = row.get("signal", 0)
        
        # 检查风险限制
        current_portfolio = engine.cash + engine.shares * price
        if not engine.check_drawdown(current_portfolio):
            print(f"  ⚠ {date.date()}: 最大回撤超限，暂停交易")
            continue
        
        if not engine.check_daily_loss(current_portfolio, prev_portfolio):
            print(f"  ⚠ {date.date()}: 单日亏损超限，暂停交易")
            continue
        
        # 执行交易
        trade = engine.execute_trade(date, price, signal, prev_portfolio)
        if trade:
            trades.append(trade)
        
        # 更新资产净值
        engine.update_portfolio(date, price)
        prev_portfolio = current_portfolio
    
    return trades


# ─────────────────────────────────────────────
# 9. 主程序
# ─────────────────────────────────────────────
def main():
    SYMBOL = "002637"      # 招商银行
    START = "20230101"
    END = "20241231"
    INIT_CASH = 100_000.0
    
    print(f"\n[1] 获取 {SYMBOL} 日K数据 {START}~{END} ...")
    df = get_daily_data(SYMBOL, START, END)
    print(f"    共 {len(df)} 个交易日")
    
    print("\n[2] 计算双均线信号（MA5 / MA20）...")
    df = add_signals(df)
    signal_count = df["signal"].abs().sum()
    print(f"    共产生 {signal_count} 个信号")
    
    print("\n[3] 样本分割（80% 训练 / 20% 测试）...")
    train_df, test_df = train_test_split(df, split_ratio=0.8)
    print(f"    训练集：{len(train_df)} 个交易日 ({train_df.index[0].date()} ~ {train_df.index[-1].date()})")
    print(f"    测试集：{len(test_df)} 个交易日 ({test_df.index[0].date()} ~ {test_df.index[-1].date()})")
    
    print("\n[4] 训练集回测...")
    engine_train = PaperTradingEngine(INIT_CASH, Config())
    trades_train = backtest(train_df, engine_train)
    analyzer_train = PerformanceAnalyzer(trades_train, engine_train.portfolio_value, INIT_CASH)
    analyzer_train.print_report("训练集回测结果")
    
    print("[5] 测试集回测（样本外验证）...")
    engine_test = PaperTradingEngine(INIT_CASH, Config())
    trades_test = backtest(test_df, engine_test)
    analyzer_test = PerformanceAnalyzer(trades_test, engine_test.portfolio_value, INIT_CASH)
    analyzer_test.print_report("测试集回测结果")
    
    print("[6] 完整数据集回测...")
    engine_full = PaperTradingEngine(INIT_CASH, Config())
    trades_full = backtest(df, engine_full)
    analyzer_full = PerformanceAnalyzer(trades_full, engine_full.portfolio_value, INIT_CASH)
    analyzer_full.print_report("完整回测结果")
    
    if trades_full:
        print("[7] 交易记录（最后10笔）:")
        trades_df = pd.DataFrame(trades_full[-10:])
        print(trades_df.to_string(index=False))

    print("\n[8] Walk-Forward 滚动验证（8段）...")
    wf_summary = walk_forward_validation(df, n_splits=8, train_ratio=0.7,
                                         init_cash=INIT_CASH)


if __name__ == "__main__":
    main()
