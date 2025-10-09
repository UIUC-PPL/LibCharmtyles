#pragma once

#include <json.hpp>
#include <string>
#include <fstream>
#include <charmtyles/util/singleton.hpp>

namespace ct {
    enum class Operations{
        EXP,
        LOG,
        ABS,
        NEGATE,
        SQUARE,
        SQRT,
        RECIPROCAL,
        SIN,
        COS,
        RELU,
        SCALE,
        ADD_CONSTANT,
        ADD,
        SUBTRACT,
        MULTIPLY,
        DIVIDE,
        POWER,
        MODULO,
        MAX,
        MIN,
        GREATER_THAN,
        LESS_THAN,
        EQUAL,
        ATAN2,
        WEIGHTED_AVERAGE
    };
    struct OperationDescriptor{
        public:
        Operations opEnum;
        std::vector<double> params;
        OperationDescriptor()=default;
        OperationDescriptor(Operations op_,std::vector<double> params_):opEnum(op_),params(params_) {}
        void pup(PUP::er& p){
            p|opEnum;
            p|params;
        }
    };

    inline OperationDescriptor add()               { return OperationDescriptor {Operations::ADD,{}}; }
    inline OperationDescriptor sub()               { return OperationDescriptor {Operations::SUBTRACT,{}}; }
    inline OperationDescriptor mul()               { return OperationDescriptor {Operations::MULTIPLY,{}}; }
    inline OperationDescriptor div()               { return OperationDescriptor {Operations::DIVIDE,{}}; }
    inline OperationDescriptor power()             { return OperationDescriptor {Operations::POWER,{}}; }
    inline OperationDescriptor modulo()            { return OperationDescriptor {Operations::MODULO,{}}; }
    inline OperationDescriptor max()               { return OperationDescriptor {Operations::MAX,{}}; }
    inline OperationDescriptor min()               { return OperationDescriptor {Operations::MIN,{}}; }
    inline OperationDescriptor greater_than()      { return OperationDescriptor {Operations::GREATER_THAN,{}}; }
    inline OperationDescriptor less_than()         { return OperationDescriptor {Operations::LESS_THAN,{}}; }
    inline OperationDescriptor equal(double epsilon) { return OperationDescriptor {Operations::EQUAL,{}}; }
    inline OperationDescriptor atan2()             { return OperationDescriptor {Operations::ATAN2,{}}; }
    inline OperationDescriptor weighted_average(double w1 ,double w2)  { return OperationDescriptor {Operations::WEIGHTED_AVERAGE,{w1,w2}}; }
    inline OperationDescriptor negate()       { return OperationDescriptor {Operations::NEGATE,{}};}
    inline OperationDescriptor abs()          { return OperationDescriptor {Operations::ABS,{}};}
    inline OperationDescriptor square()       { return OperationDescriptor {Operations::SQUARE,{}};}
    inline OperationDescriptor sqrt()         { return OperationDescriptor {Operations::SQRT,{}};}
    inline OperationDescriptor reciprocal()   { return OperationDescriptor {Operations::RECIPROCAL,{}};}
    inline OperationDescriptor sin()          { return OperationDescriptor {Operations::SIN,{}};}
    inline OperationDescriptor cos()          { return OperationDescriptor {Operations::COS,{}};}
    inline OperationDescriptor log()          { return OperationDescriptor {Operations::LOG,{}};}
    inline OperationDescriptor exp()          { return OperationDescriptor {Operations::EXP,{}};}
    inline OperationDescriptor relu()         { return OperationDescriptor {Operations::RELU,{}}; }
    inline OperationDescriptor scale(double factor)        { return OperationDescriptor {Operations::SCALE,{factor}};}
    inline OperationDescriptor add_constant(double constant) { return OperationDescriptor {Operations::ADD_CONSTANT,{constant}};}

    class ops_info_t {
    public:
        using json = nlohmann::json;

        std::string opsToFuncName(Operations op){
            return operationMap[std::to_string(1)][static_cast<int>(op)]["name"];
        }
        std::string opsToFuncDecl(Operations op, short dim) {
            return  operationMap[std::to_string(dim)][static_cast<int>(op)]["impl"];
        }

        ops_info_t() {
            std::ifstream f(std::string(BASE_DIR) + "/charmtyles/frontend/Operations.json");
            if (!f) {
                CmiAbort("Failed to open Operations.json");
            }
            operationMap = json::parse(f);
        }

    private:
        json operationMap;
    };

    CT_GENERATE_SINGLETON(ops_info_t, ops_info);
}
