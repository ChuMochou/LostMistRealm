#ifndef AIMANAGER_H
#define AIMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>

/**
 * @brief AI助手管理器
 *
 * 负责与阿里云百炼（Qwen）大模型进行异步通信。
 * 使用 Qt 自带网络模块，不依赖第三方库。
 * 所有请求均为异步，不会阻塞主线程。
 */
class AIManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造 AIManager
     * @param parent 父对象
     */
    explicit AIManager(QObject *parent = nullptr);

    /**
     * @brief 设置 API Key（不写死在代码中，方便后续修改）
     * @param key 阿里百炼 API Key
     */
    void setApiKey(const QString &key);

    /**
     * @brief 设置 Base URL（可配置）
     * @param url 接口地址，例如 https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions
     */
    void setBaseUrl(const QString &url);

    /**
     * @brief 设置使用的模型名称
     * @param model 模型名称，默认 qwen-turbo
     */
    void setModel(const QString &model);

    /**
     * @brief 发送问题给 AI（异步）
     *
     * 结果通过 answerReady 信号返回；若发生错误，则通过 errorOccurred 信号返回。
     * @param question 用户问题
     */
    void ask(const QString &question);

    /**
     * @brief 设置额外的上下文信息（用于后续扩展：玩家位置、道具数量等）
     *
     * 这些信息会附加在系统提示词的末尾，让 AI 能感知游戏状态。
     * @param context 上下文字符串（可自由格式化为 JSON 或纯文本）
     */
    void setGameContext(const QString &context);

    /**
     * @brief 判断当前是否正在等待 AI 回复
     */
    bool isBusy() const;

signals:
    /**
     * @brief AI 回答就绪
     * @param answer AI 返回的回答文本
     */
    void answerReady(const QString &answer);

    /**
     * @brief 请求过程中发生错误
     * @param errorMessage 错误描述
     */
    void errorOccurred(const QString &errorMessage);

private slots:
    /**
     * @brief 处理网络响应
     * @param reply 网络响应对象
     */
    void onReplyFinished(QNetworkReply *reply);

private:
    /**
     * @brief 构建系统提示词
     *
     * 包含游戏规则说明以及可选的动态上下文信息。
     */
    QString buildSystemPrompt() const;

    /**
     * @brief 构建 JSON 请求体
     * @param question 用户问题
     * @return 序列化后的 JSON 字节数组
     */
    QByteArray buildRequestBody(const QString &question) const;

    /**
     * @brief 解析 AI 接口返回的 JSON 响应
     * @param jsonData 原始 JSON 字节数组
     * @param[out] answer 解析出的回答文本
     * @return 是否解析成功
     */
    bool parseResponse(const QByteArray &jsonData, QString &answer) const;

    // ---- 可配置成员 ----
    QString m_apiKey;      ///< 阿里百炼 API Key（由外部设置，不写死）
    QString m_baseUrl;     ///< 接口地址
    QString m_model;       ///< 模型名称

    // ---- 内部状态 ----
    QNetworkAccessManager *m_networkManager; ///< 网络请求管理器
    QString m_gameContext;  ///< 动态游戏上下文（可选）
    bool m_isBusy;          ///< 是否正在等待响应
};

#endif // AIMANAGER_H
