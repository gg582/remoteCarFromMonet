#!/bin/sh
kubectl exec -it --tty podcar --stdin -c carcon -- /bin/bash
