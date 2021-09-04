#!/bin/bash
kubectl exec -it --tty podcar --stdin -c carcon -- /bin/bash
