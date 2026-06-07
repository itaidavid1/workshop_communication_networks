Commands:
  - On Server : mlx-stud-01  
        """  
          cd ~/tcp_benchmark  
          make clean && make  
          ./server  
        """

  - On Client : mlx-stud-02  
    """  
        cd ~/tcp_benchmark  
        make clean && make  
        ./client mlx-stud-01  
    """  
