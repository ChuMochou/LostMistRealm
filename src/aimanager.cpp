#include "aimanager.h"
#include <QDebug>

// 错误提示统一文案
static const QString ERROR_MESSAGE = QStringLiteral("AI助手暂时不可用，请稍后再试。");

AIManager::AIManager(QObject *parent)
    : QObject(parent)
    , m_apiKey(QString())
    , m_baseUrl(QStringLiteral("https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions"))
    , m_model(QStringLiteral("qwen-turbo"))
    , m_networkManager(new QNetworkAccessManager(this))
    , m_gameContext(QString())
    , m_isBusy(false)
{
    // 将网络管理器的 finished 信号连接到本类的槽函数
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &AIManager::onReplyFinished);
}

void AIManager::setApiKey(const QString &key)
{
    m_apiKey = key;
}

void AIManager::setBaseUrl(const QString &url)
{
    m_baseUrl = url;
}

void AIManager::setModel(const QString &model)
{
    m_model = model;
}

void AIManager::setGameContext(const QString &context)
{
    m_gameContext = context;
}

bool AIManager::isBusy() const
{
    return m_isBusy;
}

// ---------------------------------------------------------------------------
// 构建系统提示词（固定游戏规则 + 可选动态上下文）
// ---------------------------------------------------------------------------
QString AIManager::buildSystemPrompt() const
{
    QString prompt = QStringLiteral(
        "你是《Lost Mist Realm》的游戏助手。\n"
        "只能回答本游戏相关问题。\n"
        "游戏规则如下：\n"
        "- 地图大小23×23\n"
        "- 草地可以通行\n"
        "- 树、石头、水面不可通行\n"
        "- 玩家需要寻找钥匙打开终点附近的大门\n"
        "- 地图有战争迷雾\n"
        "- 草莓用于影响最终评价\n"
        "- 炸弹可以炸毁树和石头，不能炸毁水面\n"
        "- 玩家拥有生命值\n"
        "- 可以查看成就\n"
        "- 可以查看菜单中的操作说明\n\n"
        "如果用户询问与游戏无关的问题，请统一回答：\n"
        "\"我只能回答《Lost Mist Realm》的游戏相关内容。\"\n"
        "不要回答其它知识。"
    );

    // 如果有动态上下文（玩家位置、道具、统计信息等），附加在末尾
    if (!m_gameContext.isEmpty()) {
        prompt += QStringLiteral("\n\n当前游戏状态信息：\n") + m_gameContext;
    }

    return prompt;
}

// ---------------------------------------------------------------------------
// 构建 JSON 请求体（OpenAI 兼容格式）
// ---------------------------------------------------------------------------
QByteArray AIManager::buildRequestBody(const QString &question) const
{
    /*
     * 请求格式示例：
     * {
     *   "model": "qwen-turbo",
     *   "messages": [
     *     { "role": "system", "content": "系统提示词..." },
     *     { "role": "user",   "content": "用户问题..."   }
     *   ]
     * }
     */
    QJsonObject systemMsg;
    systemMsg["role"]    = QStringLiteral("system");
    systemMsg["content"] = buildSystemPrompt();

    QJsonObject userMsg;
    userMsg["role"]    = QStringLiteral("user");
    userMsg["content"] = question;

    QJsonArray messages;
    messages.append(systemMsg);
    messages.append(userMsg);

    QJsonObject root;
    root["model"]    = m_model;
    root["messages"] = messages;

    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

// ---------------------------------------------------------------------------
// 解析 AI 接口返回的 JSON 响应
// ---------------------------------------------------------------------------
bool AIManager::parseResponse(const QByteArray &jsonData, QString &answer) const
{
    /*
     * 响应格式示例（OpenAI 兼容）：
     * {
     *   "choices": [
     *     {
     *       "message": {
     *         "role":    "assistant",
     *         "content": "AI回答内容..."
     *       }
     *     }
     *   ]
     * }
     */
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "[AIManager] JSON 解析失败:" << parseError.errorString();
        return false;
    }

    if (!doc.isObject()) {
        qWarning() << "[AIManager] JSON 根节点不是对象";
        return false;
    }

    QJsonObject root = doc.object();

    // 检查是否有错误字段
    if (root.contains("error")) {
        QJsonObject errObj = root["error"].toObject();
        QString errMsg = errObj["message"].toString();
        qWarning() << "[AIManager] 接口返回错误:" << errMsg;
        return false;
    }

    QJsonArray choices = root["choices"].toArray();
    if (choices.isEmpty()) {
        qWarning() << "[AIManager] choices 数组为空";
        return false;
    }

    QJsonObject firstChoice = choices[0].toObject();
    QJsonObject message = firstChoice["message"].toObject();
    answer = message["content"].toString();

    return !answer.isEmpty();
}

// ---------------------------------------------------------------------------
// 发送异步请求
// ---------------------------------------------------------------------------
void AIManager::ask(const QString &question)
{
    // 防止重复请求
    if (m_isBusy) {
        emit errorOccurred(QStringLiteral("正在等待AI回复，请稍后再试。"));
        return;
    }

    // 检查 API Key 是否已配置
    if (m_apiKey.isEmpty()) {
        qWarning() << "[AIManager] API Key 未配置";
        emit errorOccurred(ERROR_MESSAGE);
        return;
    }

    m_isBusy = true;

    // 构建请求
    QNetworkRequest request;
    request.setUrl(QUrl(m_baseUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization",
                         QByteArray("Bearer ") + m_apiKey.toUtf8());

    // 构建请求体
    QByteArray body = buildRequestBody(question);

    // 发送 POST 请求（异步，不阻塞主线程）
    m_networkManager->post(request, body);
}

// ---------------------------------------------------------------------------
// 处理网络响应
// ---------------------------------------------------------------------------
void AIManager::onReplyFinished(QNetworkReply *reply)
{
    m_isBusy = false;

    // 确保 reply 在使用后被释放
    reply->deleteLater();

    // 处理网络层错误
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "[AIManager] 网络错误:" << reply->errorString();
        emit errorOccurred(ERROR_MESSAGE);
        return;
    }

    // 处理 HTTP 层错误
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode < 200 || statusCode >= 300) {
        qWarning() << "[AIManager] HTTP 错误，状态码:" << statusCode;
        emit errorOccurred(ERROR_MESSAGE);
        return;
    }

    // 读取响应数据
    QByteArray responseData = reply->readAll();

    // 解析 JSON
    QString answer;
    if (!parseResponse(responseData, answer)) {
        emit errorOccurred(ERROR_MESSAGE);
        return;
    }

    // 成功：发射信号
    emit answerReady(answer);
}
