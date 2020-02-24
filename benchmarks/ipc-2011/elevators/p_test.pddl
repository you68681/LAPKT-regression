(define (problem elevators-sequencedstrips-p16_14_1)
(:domain elevators-sequencedstrips)

(:objects 
n0 n1 n2 n3 - count
p0 p1 - passenger
fast0  - fast-elevator
slow0-0 - slow-elevator
)

(:init
(next n0 n1) (next n1 n2) (next n2 n3)
(above n0 n1) (above n0 n2) (above n0 n3)
(above n1 n2) (above n1 n3)
(above n2 n3)

(lift-at fast0 n1)
(passengers fast0 n0)
(can-hold fast0 n1)
(reachable-floor fast0 n1)(reachable-floor fast0 n3)


(lift-at slow0-0 n2)
(passengers slow0-0 n0)
(can-hold slow0-0 n0) (can-hold slow0-0 n1) (can-hold slow0-0 n2) (can-hold slow0-0 n3) 
(reachable-floor slow0-0 n0)(reachable-floor slow0-0 n1)(reachable-floor slow0-0 n2)(reachable-floor slow0-0 n3)



(passenger-at p0 n1)
(passenger-at p1 n2)


(= (travel-slow n0 n1) 6) (= (travel-slow n0 n2) 13) (= (travel-slow n0 n3) 22) (= (travel-slow n1 n2) 5) (= (travel-slow n1 n3) 12)(= (travel-slow n2 n3) 6) 



(= (travel-fast n1 n3) 10) 



(= (total-cost) 0)

)

(:goal
(and
(passenger-at p0 n3)
(passenger-at p1 n0)

))

(:metric minimize (total-cost))

)
