private:
    /**
     * 构造函数
     */
    {{@name}}_t();

    /**
     * 拷贝构造
     */
    {{@name}}_t(const {{@name}}_t&);

private:
    static {{@name}}_t* _instance; // 单件指针
    krpc_t* _rpc; // RPC实现类"
