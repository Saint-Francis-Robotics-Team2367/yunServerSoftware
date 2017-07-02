def map_range(x, in_min, in_max, out_min, out_max):
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min

class Transform:

    MOTOR_MIN = 1000
    MOTOR_IDLE = 1500
    MOTOR_MAX = 2000
    L_STICK_DEADZONE = 17
    R_STICK_DEADZONE = 17

    def __init__(self,invert_left_axis,invert_right_axis):
        self.invert_left = invert_left_axis
        self.invert_right = invert_right_axis
    
    @staticmethod
    def map_range(x, in_min, in_max, out_min, out_max):
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min




    def transform(self, left_y, right_x):
        left_y = map_range(left_y, 0, 255, Transform.MOTOR_MIN, Transform.MOTOR_MAX)
        right_x = map_range(right_x, 0, 255, Transform.MOTOR_MIN, Transform.MOTOR_MAX)

        lftMtr = Transform.MOTOR_IDLE
        rghtMtr = Transform.MOTOR_IDLE
        if left_y > Transform.MOTOR_IDLE + Transform.L_STICK_DEADZONE:
           if self.invert_left:
                rghtMtr = map_range(left_y, Transform.MOTOR_IDLE + Transform.L_STICK_DEADZONE, Transform.MOTOR_MAX, Transform.MOTOR_IDLE, Transform.MOTOR_MIN)
                lftMtr = map_range(left_y, Transform.MOTOR_IDLE + Transform.L_STICK_DEADZONE, Transform.MOTOR_MAX, Transform.MOTOR_IDLE, Transform.MOTOR_MAX)
           else:
                lftMtr = map_range(left_y, Transform.MOTOR_IDLE + Transform.L_STICK_DEADZONE, Transform.MOTOR_MAX, Transform.MOTOR_IDLE, Transform.MOTOR_MIN)
                rghtMtr = map_range(left_y, Transform.MOTOR_IDLE + Transform.L_STICK_DEADZONE, Transform.MOTOR_MAX, Transform.MOTOR_IDLE, Transform.MOTOR_MAX)

        elif left_y < Transform.MOTOR_IDLE - Transform.L_STICK_DEADZONE:
            if self.invert_left:
                lftMtr = map_range(left_y, Transform.MOTOR_IDLE - Transform.L_STICK_DEADZONE, Transform.MOTOR_MIN, Transform.MOTOR_IDLE, Transform.MOTOR_MIN)
                rghtMtr = map_range(left_y, Transform.MOTOR_IDLE - Transform.L_STICK_DEADZONE, Transform.MOTOR_MIN, Transform.MOTOR_IDLE, Transform.MOTOR_MAX)
            else:
                rghtMtr = map_range(left_y, Transform.MOTOR_IDLE - Transform.L_STICK_DEADZONE, Transform.MOTOR_MIN, Transform.MOTOR_IDLE, Transform.MOTOR_MIN)
                lftMtr = map_range(left_y, Transform.MOTOR_IDLE - Transform.L_STICK_DEADZONE, Transform.MOTOR_MIN, Transform.MOTOR_IDLE, Transform.MOTOR_MAX)

        if right_x > Transform.MOTOR_IDLE + Transform.R_STICK_DEADZONE:
            turnRate = map_range(right_x, Transform.MOTOR_IDLE + Transform.R_STICK_DEADZONE, Transform.MOTOR_MAX, Transform.MOTOR_IDLE, Transform.MOTOR_MAX)
            if self.invert_right:
                rghtMtr -= turnRate - Transform.MOTOR_IDLE
                lftMtr -= turnRate - Transform.MOTOR_IDLE
            else:
                lftMtr += turnRate - Transform.MOTOR_IDLE
                rghtMtr += turnRate - Transform.MOTOR_IDLE

        elif right_x < Transform.MOTOR_IDLE - Transform.R_STICK_DEADZONE:
            turnRate = map_range(right_x, Transform.MOTOR_IDLE - Transform.R_STICK_DEADZONE, Transform.MOTOR_MIN, Transform.MOTOR_IDLE, Transform.MOTOR_MIN)
            if self.invert_right:
                rghtMtr -= turnRate - Transform.MOTOR_IDLE
                lftMtr -= turnRate - Transform.MOTOR_IDLE
            else:
                lftMtr += turnRate - Transform.MOTOR_IDLE
                rghtMtr += turnRate - Transform.MOTOR_IDLE

        if lftMtr > Transform.MOTOR_MAX:
            lftMtr = Transform.MOTOR_MAX
        elif lftMtr < Transform.MOTOR_MIN:
            lftMtr = Transform.MOTOR_MIN

        if rghtMtr > Transform.MOTOR_MAX:
            rghtMtr = Transform.MOTOR_MAX
        elif rghtMtr < Transform.MOTOR_MIN:
            rghtMtr = Transform.MOTOR_MIN

        return lftMtr,rghtMtr

t = Transform(False,False)
print t.transform(255,255)

