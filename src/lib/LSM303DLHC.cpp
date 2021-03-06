/* (c) 2014 - Erik Regla Torres
 *
 *  This file is part of LSM303DLHC.
 *
 *  LSM303DLHC is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  LSM303DLHC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with LSM303DLHC. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * LSM303DLHC.cpp
 *
 *  Created on: Jan 30, 2014
 *      Author: jvarred
 */
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include "LSM303DLHC.h"

namespace std {

LSM303DLHC::LSM303DLHC(const char *bus_path) {
    this->i2c_accelerometer_handler = open(bus_path, O_RDWR);
    this->i2c_magnetometer_handler = open(bus_path, O_RDWR);
//    cout << this->i2c_accelerometer_handler << " : accel" << endl;
//    cout << this->i2c_magnetometer_handler << " : mag" << endl;
    this->set_device(this->i2c_accelerometer_handler, ACCELEROMETER_ADDRESS);
    this->set_device(this->i2c_magnetometer_handler, MAGNETOMETER_ADDRESS);
}

int LSM303DLHC::set_device(int bus_handler, int deviceAddress) {
    int operation_result = 0;

    operation_result = ioctl(bus_handler, I2C_TENBIT, 0);
    operation_result = ioctl(bus_handler, I2C_SLAVE, deviceAddress);

    return operation_result;
}

int LSM303DLHC::readAddress(int bus, int address) {
    int operation_result = 0;
    char rx[32];
    char tx[32];

    memset(rx, 0, sizeof(rx));
    memset(tx, 0, sizeof(tx));

    tx[0] = address;
    operation_result = write(bus, tx, 1);
    if (operation_result != 1) {
        //error
    }

    operation_result = read(bus, rx, 1);

    int value = (int) rx[0];

    return value;
}

int LSM303DLHC::writeAddress(int bus, int address, int value) {
    int operation_result = 0;
    char rx[32];
    char tx[32];

    memset(rx, 0, sizeof(rx));
    memset(tx, 0, sizeof(tx));

    tx[0] = address;
    tx[1] = value;
    operation_result = write(bus, tx, 2);
    if (operation_result != 1) {
        //error
    }

    operation_result = read(bus, rx, 1);

    int value_ = (int) rx[0];

    return value_;
}

int LSM303DLHC::read_accelerometer(lsm303_t *target) {
    u_int8_t h = 0x0;
    u_int8_t l = 0x0;
    int16_t v = 0;

    h = this->readAddress(i2c_accelerometer_handler, LSM303DLHC_OUT_X_H_A);
    l = this->readAddress(i2c_accelerometer_handler, LSM303DLHC_OUT_X_L_A);
    v = ((h << 8) | l);
    target->x = v * this->accelerometer_scale;

    h = this->readAddress(i2c_accelerometer_handler, LSM303DLHC_OUT_Y_H_A);
    l = this->readAddress(i2c_accelerometer_handler, LSM303DLHC_OUT_Y_L_A);
    v = ((h << 8) | l);
    target->y = v * this->accelerometer_scale;

    h = this->readAddress(i2c_accelerometer_handler, LSM303DLHC_OUT_Z_H_A);
    l = this->readAddress(i2c_accelerometer_handler, LSM303DLHC_OUT_Z_L_A);
    v = ((h << 8) | l);
    target->z = v * this->accelerometer_scale;
//
//    target->x *= this->accelerometer_scale;
//    target->y *= this->accelerometer_scale;
//    target->z *= this->accelerometer_scale;

    return 0;
}

int LSM303DLHC::read_magnetometer(lsm303_t *target) {
    u_int8_t h = 0x0;
    u_int8_t l = 0x0;
    int16_t v = 0;

    //the average gauss value of the earth's magnetic field is 0.5 gauss... so, don't worry if you get low values.

    h = this->readAddress(i2c_magnetometer_handler, LSM303DLHC_OUT_X_H_M);
    l = this->readAddress(i2c_magnetometer_handler, LSM303DLHC_OUT_X_L_M);
    v = ((h << 8) | l);
//    cout << "MagX: " << v << " ";
    target->x = v * this->magnetometer_scale;

    h = this->readAddress(i2c_magnetometer_handler, LSM303DLHC_OUT_Y_H_M);
    l = this->readAddress(i2c_magnetometer_handler, LSM303DLHC_OUT_Y_L_M);
    v = ((h << 8) | l);
//    cout << "MagY: " << v << " ";
    target->y = v * this->magnetometer_scale;

    h = this->readAddress(i2c_magnetometer_handler, LSM303DLHC_OUT_Z_H_M);
    l = this->readAddress(i2c_magnetometer_handler, LSM303DLHC_OUT_Z_L_M);
    v = ((h << 8) | l);
//    cout << "MagZ: " << v << endl;
    target->z = v * this->magnetometer_scale;

//
//    target->x *= this->magnetometer_scale;
//    target->y *= this->magnetometer_scale;
//    target->z *= this->magnetometer_scale;

    return 0;
}

int LSM303DLHC::init_magnetometer(int speed, int gain, int conversion) {
    this->writeAddress(this->i2c_magnetometer_handler, LSM303DLHC_CRA_REG_M, speed);
    this->writeAddress(this->i2c_magnetometer_handler, LSM303DLHC_CRB_REG_M, gain);
    this->writeAddress(this->i2c_magnetometer_handler, LSM303DLHC_MR_REG_M, conversion);

    switch (gain) {
        case LSM303DLHC_GN0:
            this->magnetometer_scale = (1.3 / 2047.0);
            break;
        case LSM303DLHC_GN1:
            this->magnetometer_scale = (1.9 / 2047.0);
            break;
        case LSM303DLHC_GN1 | LSM303DLHC_GN0:
            this->magnetometer_scale = (2.5 / 2047.0);
            break;
        case LSM303DLHC_GN2:
            this->magnetometer_scale = (4.0 / 2047.0);
            break;
        case LSM303DLHC_GN2 | LSM303DLHC_GN0:
            this->magnetometer_scale = (4.7 / 2047.0);
            break;
        case LSM303DLHC_GN2 | LSM303DLHC_GN1:
            this->magnetometer_scale = (5.6 / 2047.0);
            break;
        case LSM303DLHC_GN2 | LSM303DLHC_GN1 | LSM303DLHC_GN0:
            this->magnetometer_scale = (8.1 / 2047.0);
            break;
    }

    return 1;
}

int LSM303DLHC::init_accelerometer(int power, int scale) {
    this->writeAddress(this->i2c_accelerometer_handler, LSM303DLHC_CTRL_REG1_A, power);
    this->writeAddress(this->i2c_accelerometer_handler, LSM303DLHC_CTRL_REG4_A, scale);
    switch (scale) {
        case LSM303DLHC_FS0:
            this->accelerometer_scale = (4.0 / 32768.0);
            break;
        case LSM303DLHC_FS1:
            this->accelerometer_scale = (8.0 / 32768.0);
            break;
        case LSM303DLHC_FS0 | LSM303DLHC_FS1:
            this->accelerometer_scale = (16.0 / 32768.0);
            break;

        default:
            this->accelerometer_scale = (1.0 / 32768.0);
            break;
    }

    return 1;
}

LSM303DLHC::~LSM303DLHC() {
// TODO Auto-generated destructor stub
}

} /* namespace std */
