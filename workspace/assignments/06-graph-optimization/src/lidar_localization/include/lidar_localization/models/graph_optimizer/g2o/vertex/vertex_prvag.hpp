/*
 * @Description: g2o vertex for LIO extended pose
 * @Author: Ge Yao
 * @Date: 2020-11-29 15:47:49
 */
#ifndef LIDAR_LOCALIZATION_MODELS_GRAPH_OPTIMIZER_G2O_VERTEX_VERTEX_PRVAG_HPP_
#define LIDAR_LOCALIZATION_MODELS_GRAPH_OPTIMIZER_G2O_VERTEX_VERTEX_PRVAG_HPP_

#include <iostream>

#include <Eigen/Core>
#include <Eigen/Dense>

#include <sophus/so3.hpp>

#include <g2o/core/base_vertex.h>

#include "lidar_localization/sensor_data/key_frame.hpp"

namespace g2o {

struct PRVAG {
    static const int INDEX_POS = 0;
    static const int INDEX_ORI = 3;
    static const int INDEX_VEL = 6;
    static const int INDEX_B_A = 9;
    static const int INDEX_B_G = 12;

    PRVAG() {}

    explicit PRVAG(const double *data) {
        pos = Eigen::Vector3d(data[INDEX_POS + 0], data[INDEX_POS + 1], data[INDEX_POS + 2]);
        ori = Sophus::SO3d::exp(
              Eigen::Vector3d(data[INDEX_ORI + 0], data[INDEX_ORI + 1], data[INDEX_ORI + 2])
        );
        vel = Eigen::Vector3d(data[INDEX_VEL + 0], data[INDEX_VEL + 1], data[INDEX_VEL + 2]);
        b_a = Eigen::Vector3d(data[INDEX_B_A + 0], data[INDEX_B_A + 1], data[INDEX_B_A + 2]);
        b_g = Eigen::Vector3d(data[INDEX_B_G + 0], data[INDEX_B_G + 1], data[INDEX_B_G + 2]);
    }

    explicit PRVAG(const lidar_localization::KeyFrame &key_frame) {
        pos = key_frame.pose.block<3, 1>(0, 3).cast<double>();
        vel = key_frame.vel.cast<double>();
        ori = Sophus::SO3d(
            Eigen::Quaterniond(key_frame.pose.block<3, 3>(0, 0).cast<double>())
        );
        b_a = key_frame.bias.accel.cast<double>();
        b_g = key_frame.bias.gyro.cast<double>();
    }

    void WriteTo(double *data) {
        // get orientation in so3:
        auto log_ori = ori.log();

        for (size_t i = 0; i < 3; ++i) {
            data[INDEX_POS + i] = pos(i);
            data[INDEX_ORI + i] = log_ori(i);
            data[INDEX_VEL + i] = vel(i);
            data[INDEX_B_A + i] = b_a(i);
            data[INDEX_B_G + i] = b_g(i);
        }
    }

    Eigen::Vector3d pos = Eigen::Vector3d::Zero();
    Sophus::SO3d ori = Sophus::SO3d();
    Eigen::Vector3d vel = Eigen::Vector3d::Zero();
    Eigen::Vector3d b_a = Eigen::Vector3d::Zero();
    Eigen::Vector3d b_g = Eigen::Vector3d::Zero();
};

class VertexPRVAG : public g2o::BaseVertex<15, PRVAG> {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    virtual void setToOriginImpl() override {
        _estimate = PRVAG();
    }

    virtual void oplusImpl(const double *update) override {
        _estimate.pos += Eigen::Vector3d(
            update[PRVAG::INDEX_POS + 0], update[PRVAG::INDEX_POS + 1], update[PRVAG::INDEX_POS + 2]
        );
        _estimate.ori = _estimate.ori * Sophus::SO3d::exp(
            Eigen::Vector3d(
                update[PRVAG::INDEX_ORI + 0], update[PRVAG::INDEX_ORI + 1], update[PRVAG::INDEX_ORI + 2]
            )
        );
        _estimate.vel += Eigen::Vector3d(
            update[PRVAG::INDEX_VEL + 0], update[PRVAG::INDEX_VEL + 1], update[PRVAG::INDEX_VEL + 2]
        );
        _estimate.b_a += Eigen::Vector3d(
            update[PRVAG::INDEX_B_A + 0], update[PRVAG::INDEX_B_A + 1], update[PRVAG::INDEX_B_A + 2]
        );
        _estimate.b_g += Eigen::Vector3d(
            update[PRVAG::INDEX_B_G + 0], update[PRVAG::INDEX_B_G + 1], update[PRVAG::INDEX_B_G + 2]
        );
    }

    virtual bool read(std::istream &in) { return true; }

    virtual bool write(std::ostream &out) const { return true; }
};

} // namespace g2o

#endif // LIDAR_LOCALIZATION_MODELS_GRAPH_OPTIMIZER_G2O_VERTEX_VERTEX_PRVAG_HPP_