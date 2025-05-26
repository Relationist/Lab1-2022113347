import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.PriorityQueue;
import java.util.Random;
import java.util.Scanner;
import java.util.Set;

/**
 * GraphTool 是一个基于有向图的文本处理工具类，支持以下功能：
 * <ul>
 *   <li>构建文本对应的词图</li>
 *   <li>查找桥接词</li>
 *   <li>生成带桥接词的新文本</li>
 *   <li>计算两个词之间的最短路径</li>
 *   <li>计算图中各节点的 PageRank</li>
 *   <li>执行随机游走并输出路径</li>
 *   <li>生成 DOT 文件和 PNG 图像以可视化图结构</li>
 * </ul>
 * 用户可通过命令行交互菜单选择并执行各项功能.
 */

@SuppressWarnings("checkstyle:SummaryJavadoc")
public class GraphTool {
  static class Graph {
    Map<String, Map<String, Integer>> adj = new HashMap<>();
    private final Random random = new Random();
    void addEdge(String from, String to) {
      adj.computeIfAbsent(from, k -> new HashMap<>()).merge(to, 1, Integer::sum);
      adj.putIfAbsent(to, new HashMap<>());
    }

    boolean hasWord(String word) {
      return adj.containsKey(word);
    }

    List<String> findBridgeWords(String w1, String w2) {
      List<String> bridges = new ArrayList<>();
      if (!hasWord(w1) || !hasWord(w2)) {
        return bridges;
      }
      for (String mid : adj.getOrDefault(w1, Map.of()).keySet()) {
        if (adj.getOrDefault(mid, Map.of()).containsKey(w2)) {
          bridges.add(mid);
        }
      }
      return bridges;
    }

    String generateNewText(String text) {
      List<String> words = tokenize(cleanText(text));
      if (words.isEmpty()) {
        return "";
      }
      StringBuilder sb = new StringBuilder(words.get(0));
      for (int i = 1; i < words.size(); i++) {
        String w1 = words.get(i - 1);
        String w2 = words.get(i);
        List<String> bridges = findBridgeWords(w1, w2);
        if (!bridges.isEmpty()) {
          String bridge = bridges.get(random.nextInt(bridges.size())); // 复用random
          sb.append(" ").append(bridge);
        }
        sb.append(" ").append(w2);
      }
      return sb.toString();
    }

    Map<String, Integer> dijkstra(String start, Map<String, String> prev) {
      Map<String, Integer> dist = new HashMap<>();
      for (String node : adj.keySet()) {
        dist.put(node, Integer.MAX_VALUE);
      }
      if (!adj.containsKey(start)) {
        return dist;
      }
      dist.put(start, 0);
      PriorityQueue<String> pq = new PriorityQueue<>(Comparator.comparingInt(dist::get));
      pq.add(start);
      while (!pq.isEmpty()) {
        String u = pq.poll();
        for (Map.Entry<String, Integer> neighbor : adj.getOrDefault(u, Map.of()).entrySet()) {
          String v = neighbor.getKey();
          int alt = dist.get(u) + neighbor.getValue();
          if (alt < dist.get(v)) {
            dist.put(v, alt);
            prev.put(v, u);
            pq.add(v);
          }
        }
      }
      return dist;
    }

    List<String> shortestPath(String start, String end) {
      Map<String, String> prev = new HashMap<>();
      Map<String, Integer> dist = dijkstra(start, prev);
      List<String> path = new ArrayList<>();
      if (!dist.containsKey(end) || dist.get(end) == Integer.MAX_VALUE) {
        return path;
      }
      for (String at = end; at != null; at = prev.get(at)) {
        path.add(at);
      }
      Collections.reverse(path);
      return path;
    }

    Map<String, Double> pageRank(double d, int iter) {
      Map<String, Double> pr = new HashMap<>();
      int n = adj.size();
      double init = 1.0 / n;
      for (String node : adj.keySet()) {
        pr.put(node, init);
      }

      for (int i = 0; i < iter; i++) {
        Map<String, Double> newPr = new HashMap<>();
        double danglingSum = 0.0;

        for (Map.Entry<String, Map<String, Integer>> entry : adj.entrySet()) {
          if (entry.getValue().isEmpty()) {
            danglingSum += pr.get(entry.getKey());
          }
        }

        for (String node : adj.keySet()) {
          newPr.put(node, (1 - d) / n + d * danglingSum / n);
        }
        for (Map.Entry<String, Map<String, Integer>> entry : adj.entrySet()) {
          String node = entry.getKey();
          Map<String, Integer> neighbors = entry.getValue();
          if (!neighbors.isEmpty()) {
            double share = pr.get(node) / neighbors.size();
            for (Map.Entry<String, Integer> neiEntry : neighbors.entrySet()) {
              String nei = neiEntry.getKey();
              newPr.put(nei, newPr.get(nei) + d * share);
            }
          }
        }
        pr = newPr;
      }
      return pr;
    }

    List<String> randomWalk() {
      List<String> walk = new ArrayList<>();
      if (adj.isEmpty()) {
        return walk;
      }
      Random rand = new Random();
      List<String> nodes = new ArrayList<>(adj.keySet());
      String current = nodes.get(rand.nextInt(nodes.size()));
      walk.add(current);
      Set<String> visitedEdges = new HashSet<>();
      while (true) {
        Map<String, Integer> neighbors = adj.getOrDefault(current, Map.of());
        if (neighbors.isEmpty()) {
          break;
        }
        List<String> options = new ArrayList<>(neighbors.keySet());
        String next = options.get(rand.nextInt(options.size()));
        String edge = current + "->" + next;
        if (visitedEdges.contains(edge)) {
          break;
        }
        visitedEdges.add(edge);
        walk.add(next);
        current = next;
      }
      return walk;
    }

    String generateDot() {
      StringBuilder sb = new StringBuilder("digraph G {\n");
      for (var entry : adj.entrySet()) {
        for (var to : entry.getValue().entrySet()) {
          sb.append("    \"").append(entry.getKey()).append("\" -> \"")
              .append(to.getKey()).append("\" [label=\"")
              .append(to.getValue()).append("\"];\n");
        }
      }
      sb.append("}\n");
      return sb.toString();
    }

    void saveDot(String filename) throws IOException {
      Files.writeString(Paths.get(filename), generateDot());
      try {
        // 生成图片命令
        ProcessBuilder pbDot = new ProcessBuilder(
            "C:\\Program Files\\Graphviz\\bin\\dot.exe",
            "-Tpng",
            "C:\\Users\\16586\\IdeaProjects\\lab3_521\\graph.dot",
            "-o",
            "C:\\Users\\16586\\IdeaProjects\\lab3_521\\graph.png"
        );
        pbDot.redirectErrorStream(true);
        Process pDot = pbDot.start();
        int exitCode = pDot.waitFor();

        if (exitCode == 0) {
          System.out.println("图片生成成功！");
          // 打开图片命令
          ProcessBuilder pbOpen = new ProcessBuilder(
              "cmd", "/c", "start", "\"\"", "C:\\Users\\16586\\IdeaProjects\\lab3_521\\graph.png");
          pbOpen.start();
        } else {
          System.err.println("生成图片失败，退出码：" + exitCode);
        }
      } catch (Exception e) {
        e.printStackTrace();
      }

    }
  }

  static String cleanText(String text) {
    return text.replaceAll("[^a-zA-Z]", " ").toLowerCase();
  }

  static List<String> tokenize(String text) {
    return Arrays.stream(text.split("\\s+")).filter(s -> !s.isEmpty()).toList();
  }

  static Graph buildGraph(String filename) throws IOException {
    Graph g = new Graph();
    String content = Files.readString(Paths.get(filename));
    List<String> words = tokenize(cleanText(content));
    for (int i = 1; i < words.size(); i++) {
      g.addEdge(words.get(i - 1), words.get(i));
    }
    return g;
  }
  /**
   * 程序入口方法，提供命令行菜单界面，供用户选择图相关功能进行操作：
   * 展示图、查询桥接词、生成新文本、查询最短路径、计算 PageRank、随机游走等。
   *
   * @param args 命令行参数（未使用）
   * @throws IOException 文件读取或写入过程中可能抛出的异常
   */

  public static void main(String[] args) throws IOException {
    Scanner sc = new Scanner(System.in, StandardCharsets.UTF_8);
    String file = "src/input.txt";
    Graph graph = buildGraph(file);


    while (true) {
      System.out.println(
          "\n功能菜单:\n1. 展示图\n2. 查询桥接词\n3. 生成新文本\n4. 查询最短路径\n5. 计算PageRank\n6. 随机游走\n0. 退出");
      System.out.print("请输入功能编号：");
      String choice = sc.nextLine();
      switch (choice) {
        case "0" -> System.exit(0);
        case "1" -> {
          graph.saveDot("graph.dot");
          System.out.println("Graphviz 文件已生成：graph.dot");
        }
        case "2" -> {
          System.out.print("输入两个单词：");
          String[] parts = sc.nextLine().split(" ");
          List<String> bridges = graph.findBridgeWords(parts[0], parts[1]);
          if (bridges.isEmpty()) {
            System.out.println("No bridge words from " + parts[0] + " to " + parts[1]);
          } else {
            System.out.println("桥接词: " + String.join(", ", bridges));
          }
        }
        case "3" -> {
          System.out.print("输入一句文本：");
          String input = sc.nextLine();
          System.out.println("生成的新文本: " + graph.generateNewText(input));
        }
        case "4" -> {
          System.out.print("输入一个或两个单词：");
          String[] words = sc.nextLine().split(" ");
          if (words.length == 1) {
            Map<String, String> prev = new HashMap<>();
            Map<String, Integer> dist = graph.dijkstra(words[0], prev);
            for (Map.Entry<String, Integer> entry : dist.entrySet()) {
              String key = entry.getKey();
              int distance = entry.getValue();
              if (key.equals(words[0]) || distance == Integer.MAX_VALUE) {
                continue;
              }
              List<String> path = graph.shortestPath(words[0], key);
              System.out.println("到 " + key + " 的路径: " + String.join(" -> ", path) + "，长度: " + distance);
            }

          } else if (words.length == 2) {
            List<String> path = graph.shortestPath(words[0], words[1]);
            if (path.isEmpty()) {
              System.out.println("无路径");
            } else {
              System.out.println("最短路径: " + String.join(" -> ", path));
            }
          }
        }
        case "5" -> {
          var pr = graph.pageRank(0.85, 100);
          pr.forEach((k, v) -> System.out.printf("%s: %.4f%n", k, v));
        }
        case "6" -> {
          var walk = graph.randomWalk();
          System.out.println("随机游走: " + String.join(" ", walk));
        }
        default -> System.out.println("无效选项！");
      }
    }
  }
}
