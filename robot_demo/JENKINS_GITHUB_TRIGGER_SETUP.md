# Jenkins GitHub 自动触发配置指南

## 概述

本文档说明如何配置 Jenkins 使其在 GitHub 代码提交后自动触发 CI/CD Job。

---

## 配置步骤

### 1️⃣ GitHub 端配置（设置 Webhook）

在你的 GitHub 仓库中完成以下操作：

1. 进入仓库 **Settings** 页面
2. 左侧菜单选择 **Webhooks** → **Add webhook**
3. 填写以下内容：
   - **Payload URL**: `http://your-jenkins-server/github-webhook/`
   - **Content type**: `application/json`
   - **Which events**: 
     - 选择 `Push events` （代码提交触发）
     - 或 `Pull requests` （拉取请求触发）
     - 或 `Just the push event` 等其他选项
   - **Active**: ✅ 勾选
4. 点击 **Add webhook** 保存

**测试 Webhook**：
- 在 Webhook 页面可以看到最近的 Delivery 记录
- 如果显示绿色勾号表示连接成功
- 点击可查看 Request/Response 详情进行调试

---

### 2️⃣ Jenkins 插件安装

Jenkins 需要安装以下插件以支持 GitHub 集成：

| 插件名称 | 用途 |
|---------|------|
| **GitHub plugin** | 基础 GitHub 集成 |
| **GitHub Branch Source** | 支持 webhook 触发 |
| **GitHub Integration Plugin** | 增强集成功能 |

**安装步骤**：

1. 在 Jenkins 中进入 **Manage Jenkins** → **Plugin Manager**
2. 点击 **Available** 标签页
3. 搜索上述插件名称
4. 勾选后点击 **Install** 
5. 重启 Jenkins

---

### 3️⃣ Jenkins 中配置 GitHub 凭证

1. 进入 **Manage Jenkins** → **Manage Credentials**
2. 选择 **Jenkins** 存储域
3. 点击 **Add Credentials**
4. 填写信息：
   - **Kind**: 选择 `GitHub App` 或 `Username with password`
   - **Username**: GitHub 用户名
   - **Personal Access Token**: [从 GitHub 生成](https://github.com/settings/tokens)
     - 权限范围：`repo`, `admin:repo_hook`, `admin:org_hook`
   - **ID**: `github-credentials` （供后续引用）
5. 点击 **Create** 保存

---

### 4️⃣ 创建/修改 Jenkinsfile

在 GitHub 仓库根目录（或指定目录）创建 `Jenkinsfile` 文件：

```groovy
pipeline {
    agent any

    options {
        timestamps()
        timeout(time: 1, unit: 'HOURS')
        buildDiscarder(logRotator(numToKeepStr: '10'))
    }

    triggers {
        githubPush()  // GitHub Webhook 自动触发
    }

    environment {
        WORKSPACE_SETUP = "${WORKSPACE}/setup"
    }

    stages {
        stage('Checkout') {
            steps {
                checkout scm
                script {
                    echo "✅ Repository checked out"
                    echo "Branch: ${GIT_BRANCH}"
                    echo "Commit: ${GIT_COMMIT}"
                }
            }
        }

        stage('Setup Environment') {
            steps {
                sh '''
                    echo "Setting up build environment..."
                    python3 --version || true
                    pwd
                    ls -la
                '''
            }
        }

        stage('Configure') {
            steps {
                sh '''
                    # 根据需要执行配置命令
                    echo "Configuring application..."
                    # make configure_app ARCH=x86_64 OPTIMIZATION=no
                '''
            }
        }

        stage('Build') {
            steps {
                sh '''
                    echo "Building..."
                    # make buildut ARCH=x86_64
                    echo "Build completed"
                '''
            }
        }

        stage('Test') {
            steps {
                sh '''
                    echo "Running tests..."
                    # ctest --output-on-failure
                    echo "Tests completed"
                '''
            }
        }

        stage('Archive Results') {
            steps {
                script {
                    writeFile file: 'build-summary.txt', text: """
Build Summary
=============
Status: SUCCESS
Branch: ${GIT_BRANCH}
Commit: ${GIT_COMMIT}
Timestamp: ${env.BUILD_TIMESTAMP}
"""
                    archiveArtifacts artifacts: 'build-summary.txt', fingerprint: true
                }
            }
        }
    }

    post {
        success {
            echo '✅ Pipeline succeeded'
        }
        failure {
            echo '❌ Pipeline failed'
        }
        always {
            echo '⏹️  Pipeline finished'
            cleanWs()  // 清理工作空间
        }
    }
}
```

---

### 5️⃣ Jenkins Job 配置

#### 方式 A：创建新的 Pipeline Job

1. Jenkins 首页 → **New Item**
2. 输入 Job 名称，选择 **Pipeline** 类型
3. 点击 **OK** 进入配置页面
4. 在 **Pipeline** 部分选择：
   - **Definition**: `Pipeline script from SCM`
   - **SCM**: `Git`
   - **Repository URL**: 
     ```
     https://github.com/your-org/your-repo.git
     ```
   - **Credentials**: 选择前面创建的 GitHub 凭证
   - **Branch Specifier**: `*/main` 或 `*/develop`
   - **Script Path**: `Jenkinsfile` （根目录）或 `path/to/Jenkinsfile`

5. **Build Triggers** 部分勾选：
   - `GitHub hook trigger for GITScm polling`

6. 点击 **Save** 保存

#### 方式 B：Multibranch Pipeline Job

1. Jenkins 首页 → **New Item**
2. 输入 Job 名称，选择 **Multibranch Pipeline** 类型
3. 在 **Branch Sources** 中：
   - 点击 **Add source** → 选择 `Git`
   - **Repository URL**: GitHub 仓库地址
   - **Credentials**: 选择 GitHub 凭证
4. 在 **Scan Multibranch Pipeline Triggers** 中：
   - 勾选 `Periodically if not otherwise run` （备用轮询）
   - 设置扫描间隔（如 1 hour）
5. 点击 **Save**

---

### 6️⃣ 验证配置

#### 方式 1：通过 GitHub Webhook 测试

1. 进入 GitHub 仓库 → **Settings** → **Webhooks**
2. 找到添加的 Webhook，点击 **Edit**
3. 滚动到 **Recent Deliveries** 部分
4. 查看最近的请求记录
5. 点击记录查看 **Request** 和 **Response** 详情
6. 如果 Response 状态码为 200，表示配置成功

#### 方式 2：手动测试

```bash
# 测试 Jenkins Webhook 端点
curl -X POST http://your-jenkins-server:8080/github-webhook/ \
  -H "Content-Type: application/json" \
  -H "X-GitHub-Event: push" \
  -d '{
    "action":"opened",
    "repository": {"url": "https://github.com/your-org/your-repo"},
    "push": true
  }'
```

#### 方式 3：实际测试

1. 在本地修改代码
2. 提交并推送到 GitHub：
   ```bash
   git add .
   git commit -m "test jenkins trigger"
   git push origin main
   ```
3. 进入 Jenkins Job 页面，观察是否自动触发新的 Build
4. 检查 **Console Output** 查看执行过程

---

## 常见问题排查

### ❌ Webhook 返回 403 Forbidden

**原因**：Jenkins 未正确配置 GitHub 凭证或权限不足

**解决**：
1. 检查 GitHub Personal Access Token 权限
2. 确保勾选了 `repo` 和 `admin:repo_hook` 权限
3. 重新生成 Token 并更新 Jenkins 凭证

### ❌ Webhook 连接超时或返回 504

**原因**：Jenkins 服务器不可达或防火墙阻止

**解决**：
1. 确保 Jenkins 服务器地址可公开访问
2. 检查防火墙规则
3. 查看 Jenkins 日志：`tail -f $JENKINS_HOME/logs/jenkins.log`

### ❌ Job 未自动触发

**原因**：
- Jenkinsfile 中缺少 `triggers { githubPush() }`
- GitHub plugin 未安装
- Webhook 配置错误

**解决**：
1. 确认 Jenkinsfile 包含触发器配置
2. 重新安装 GitHub plugin
3. 重新配置 Webhook，确保 Payload URL 正确
4. 查看 Jenkins 中 Job 的 **Configure** → **Build Triggers** 是否勾选

### ❌ Jenkinsfile 语法错误

**排查**：
1. 在 Jenkins 中 **Pipeline Syntax** 工具验证
2. 使用 [Jenkinsfile Linter](https://www.jenkins.io/blog/2016/05/16/jenkinsfile-linter/) 在线检查
3. 本地使用 `groovy` 命令验证（如果安装了）

---

## 安全建议

| 项目 | 建议 |
|-----|------|
| **Token 管理** | 定期更换 Personal Access Token |
| **权限最小化** | 只授予必要的 GitHub 权限 |
| **凭证存储** | 使用 Jenkins Credentials Store，不要硬编码 |
| **Webhook 验证** | GitHub 会发送 `X-Hub-Signature-256` 头用于验证来源 |
| **网络隔离** | 限制 GitHub 服务器 IP 访问 Jenkins（可选） |

---

## 完整工作流程图

```
GitHub Push
    ↓
GitHub 发送 Webhook 请求
    ↓
Jenkins 接收 Webhook
    ↓
触发 Pipeline Job
    ↓
Jenkins Clone 代码
    ↓
Jenkins 执行 Jenkinsfile
    ↓
执行各个 Stage（Configure → Build → Test）
    ↓
生成构建结果和日志
    ↓
推送通知（可选：邮件、Slack 等）
```

---
GitHub 发 Webhook 是指：当代码仓库（如 push、PR 等事件）发生指定动作时，GitHub 主动向你预设的 HTTP URL 发送 POST 请求（含事件数据），实现“事件推送”而非轮询。‌‌

‌Jenkins 与 GitHub Webhook 关联的核心是：在 GitHub 配置 Webhook 指向 Jenkins 的 /github-webhook/ 接口，并在 Jenkins 任务中启用“GitHub hook trigger for GITScm polling”，从而实现代码提交后自动触发构建。‌‌‌

‌GitHub 配置 Webhook‌：进入仓库 → ‌Settings → Webhooks → Add webhook‌，Payload URL 填 http(s)://<你的Jenkins地址>/github-webhook/（需公网可访问），Content type 选 application/json，选择触发事件（如 Just the push event）。
‌Jenkins 配置接收‌：安装 ‌GitHub Plugin‌；在任务配置 → ‌Build Triggers‌ → 勾选 ‌“GitHub hook trigger for GITScm polling”‌；确保 Jenkins 未被防火墙拦截，且无须认证（或配置 Webhook 密钥验证）。
‌验证生效‌：提交代码后，查看 GitHub Webhook 的 ‌Recent Deliveries‌ 是否有 200 响应；Jenkins 日志应显示“GitHub push event received”。‌‌
若 Jenkins 在内网，需用 ‌ngrok‌ 或反向代理暴露地址；建议启用 ‌Webhook Secret‌（在 GitHub 和 Jenkins 插件中一致配置）提升安全。不依赖 Generic Webhook Trigger 插件时，必须用 /github-webhook/ 路径且启用对应插件。
---
## 参考资源

- [Jenkins GitHub Plugin](https://plugins.jenkins.io/github/)
- [GitHub Webhooks Documentation](https://docs.github.com/en/developers/webhooks-and-events/webhooks)
- [Jenkinsfile Syntax](https://www.jenkins.io/doc/book/pipeline/jenkinsfile/)
- [Personal Access Token 生成](https://github.com/settings/tokens)

---

## 更新日期

- **创建日期**: 2026-05-27
- **最后更新**: 2026-05-27
