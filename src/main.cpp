#include <math.h>
#include <uWS/uWS.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include "Eigen-3.3/Eigen/Core"
#include "Eigen-3.3/Eigen/QR"
#include "MPC.h"
#include "json.hpp"


// Some of the codes are inspired from this link
// https://github.com/mvirgo/MPC-Project/blob/master/src/main.cpp
// for convenience
using json = nlohmann::json;

// For converting back and forth between radians and degrees.
constexpr double pi() { return M_PI; }
double deg2rad(double x) { return x * pi() / 180; }
double rad2deg(double x) { return x * 180 / pi(); }

// Checks if the SocketIO event has JSON data.
// If there is data the JSON object in string format will be returned,
// else the empty string "" will be returned.
string hasData(string s) {
  auto found_null = s.find("null");
  auto b1 = s.find_first_of("[");
  auto b2 = s.rfind("}]");
  if (found_null != string::npos) {
    return "";
  } else if (b1 != string::npos && b2 != string::npos) {
    return s.substr(b1, b2 - b1 + 2);
  }
  return "";
}

// Evaluate a polynomial.
double polyeval(Eigen::VectorXd coeffs, double x) {
  double result = 0.0;
  for (int i = 0; i < coeffs.size(); i++) {
    result += coeffs[i] * pow(x, i);
  }
  return result;
}

// Fit a polynomial.
// Adapted from
// https://github.com/JuliaMath/Polynomials.jl/blob/master/src/Polynomials.jl#L676-L716
Eigen::VectorXd polyfit(Eigen::VectorXd xvals, Eigen::VectorXd yvals,
                        int order) {
  assert(xvals.size() == yvals.size());
  assert(order >= 1 && order <= xvals.size() - 1);
  Eigen::MatrixXd A(xvals.size(), order + 1);

  for (int i = 0; i < xvals.size(); i++) {
    A(i, 0) = 1.0;
  }

  for (int j = 0; j < xvals.size(); j++) {
    for (int i = 0; i < order; i++) {
      A(j, i + 1) = A(j, i) * xvals(j);
    }
  }

  auto Q = A.householderQr();
  auto result = Q.solve(yvals);
  return result;
}

int main() {
  uWS::Hub h;

  // MPC is initialized here!
  MPC mpc;

  h.onMessage([&mpc](uWS::WebSocket<uWS::SERVER> ws, char *data, size_t length,
                     uWS::OpCode opCode) {
    // "42" at the start of the message means there's a websocket message event.
    // The 4 signifies a websocket message
    // The 2 signifies a websocket event
    string sdata = string(data).substr(0, length);
    cout << sdata << endl;
    if (sdata.size() > 2 && sdata[0] == '4' && sdata[1] == '2') {
      string s = hasData(sdata);
      if (s != "") {
        auto j = json::parse(s);
        string event = j[0].get<string>();
        if (event == "telemetry") {
          // j[1] is the data JSON object
          vector<double> ptsx = j[1]["ptsx"];
          vector<double> ptsy = j[1]["ptsy"];
          double px = j[1]["x"];
          double py = j[1]["y"];
          double psi = j[1]["psi"];
          double v = j[1]["speed"];
          double steering = j[1]["steering_angle"];
          double throt = j[1]["throttle"];

          /*
          * TODO: Calculate steering angle and throttle using MPC.
          *
          * Both are in between [-1, 1].
          *
          */
          
          // Need Eigen vectors for polyfit
          Eigen::VectorXd veh_x(ptsx.size());
          Eigen::VectorXd veh_y(ptsy.size());
          
          // Going to the vehicle coordinates
          for (int i = 0; i < ptsx.size(); i++) {
            double dx = ptsx[i] - px;
            double dy = ptsy[i] - py;
            veh_x[i] = dx * cos(-psi) - dy * sin(-psi);
            veh_y[i] = dx * sin(-psi) + dy * cos(-psi);
          }
          
          /*
          * Calculate steering angle and throttle using MPC.
          * Both are in between [-1, 1].
          * Simulator has 100ms latency, so will predict state at that point in time.
          * This will help the car react to where it is actually at by the point of actuation.
          */
          
          // Fits a 3rd-order polynomial to the above x and y coordinates
          auto coeffs = polyfit(veh_x, veh_y, 3);
          
          // Calculates the cross track error
          // Because points were transformed to vehicle coordinates, x & y equal 0 below.
          // 'y' would otherwise be subtracted from the polyeval value
          double cte = polyeval(coeffs, 0);
          
          // Calculate the orientation error
          // Derivative of the polyfit goes in atan() below
          // Because x = 0 in the vehicle coordinates, the higher orders are zero
          // Leaves only coeffs[1]
          double epsi = -atan(coeffs[1]);
          
                    
          // dt = 0.1 is considered for the latency
          const double dt = 0.1;
          const double Lf = 2.67;
          double px_est = 0.0 + v * dt;
          const double py_est = 0.0;
          double psi_est = 0.0 + v * -steering / Lf * dt;
          double v_est = v + throt * dt;
          double cte_est = cte + v * sin(epsi) * dt;
          double epsi_est = epsi + v * -steering / Lf * dt;
          
          // Feed in the predicted state values
          Eigen::VectorXd state(6);
          state << px_est, py_est, psi_est, v_est, cte_est, epsi_est;
          
          // Solve for new actuations (and to show predicted x and y in the future)
          auto vars = mpc.Solve(state, coeffs);
          
          // Transfer the steering into the range of [-1,1]
          double steer_value = vars[0] / (deg2rad(25) * Lf);
          double throttle_value = vars[1];
          
          // Send values to the simulator
          json msgJson;
          msgJson["steering_angle"] = -steer_value;
          msgJson["throttle"] = throttle_value;

          // Predicted X and Y points
          vector<double> mpcX_pred = {state[0]};
          vector<double> mpcY_pred = {state[1]};
          for (int i = 2; i < vars.size(); i+=2) {
            mpcX_pred.push_back(vars[i]);
            mpcY_pred.push_back(vars[i+1]);
          }

          msgJson["mpc_x"] = mpcX_pred;
          msgJson["mpc_y"] = mpcY_pred;

          // Display the waypoints/reference line
          vector<double> next_x_vals;
          vector<double> next_y_vals;
          double degree = 3;
          int num_points = 20;
          
          for (int i = 1; i < num_points; i++) {
            next_x_vals.push_back(degree * i);
            next_y_vals.push_back(polyeval(coeffs, degree * i));
          }
          
          msgJson["next_x"] = next_x_vals;
          msgJson["next_y"] = next_y_vals;

          auto msg = "42[\"steer\"," + msgJson.dump() + "]";
          std::cout << msg << std::endl;
          // Latency
          // The purpose is to mimic real driving conditions where
          // the car doesn't actuate the commands instantly.
          this_thread::sleep_for(chrono::milliseconds(100));
          ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);

        }
      } else {
        // Manual driving
        std::string msg = "42[\"manual\",{}]";
        ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
      }
    }
  });

  // We don't need this since we're not using HTTP but if it's removed the
  // program
  // doesn't compile :-(
  h.onHttpRequest([](uWS::HttpResponse *res, uWS::HttpRequest req, char *data,
                     size_t, size_t) {
    const std::string s = "<h1>Hello world!</h1>";
    if (req.getUrl().valueLength == 1) {
      res->end(s.data(), s.length());
    } else {
      // i guess this should be done more gracefully?
      res->end(nullptr, 0);
    }
  });

  h.onConnection([&h](uWS::WebSocket<uWS::SERVER> ws, uWS::HttpRequest req) {
    std::cout << "Connected!!!" << std::endl;
  });

  h.onDisconnection([&h](uWS::WebSocket<uWS::SERVER> ws, int code,
                         char *message, size_t length) {
    ws.close();
    std::cout << "Disconnected" << std::endl;
  });

  int port = 4567;
  if (h.listen(port)) {
    std::cout << "Listening to port " << port << std::endl;
  } else {
    std::cerr << "Failed to listen to port" << std::endl;
    return -1;
  }
  h.run();
}
